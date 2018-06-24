# **PyDictObject**
***

原则上关联容器的设计总会关注索引效率，实现会基于设计良好的数据结构，与C++中的map类似，map是基于RBTree实现的，RBTree是一种平衡二叉树，搜索效率为O(log2N)；python中的dict由于需要在实现中被大量使用所以对搜索效率要求很苛刻（比如会通过PyDictObject建立执行python字节码的运行环境，存放变量名和变量值的元素对），因此采用了hash table，理论上的最优情况能提供O(1)的搜索效率；最新的C++中如`unordered_map`

## **1、散列表**
研究表明散列表的装载率（已用空间和总空间的比值）超过2/3的时候，冲突的概率的就会大大增加；

解决冲突的方法很多，SGI STL使用的开链法；Python中使用的开放定址法，当发生冲突的时候通过一个二次探测函数f，计算下一个候选位置addr，如果可用则将待插入元素放到addr，如果不可用则继续使用探测函数f直到找到一个可用的位置为止；

但是多次使用探测函数从一个位置达到多个位置，这就形成来“冲突探测链”；当需要删除位于探测链中间的元素的时候，如果直接删除就永远不能到达位于探测链尾部的元素；所以使用开发定址的冲突解决方案的散列表，删除某条探测链上的元素时不能真正的删除而要进行一种“假删除”的操作；

## **2、PyDictObject**
关联容器中的一个（键、值）对称为一个entry或slot，其定义如下：
```C++
struct PyDictEntry {
  Py_ssize_t me_hash;
  PyObject* me_key;
  PyObject* me_value;
};
```
在一个PyDictObject生存变化的过程中，entry会在Unused状态、Active状态和Dummy状态之间转换；
  - entry的`me_key`和`me_value`都为NULL的时候处于Unused状态，表明该entry当前和之前都没有存储(key,value)对；
  - entry中存储了一个(key,value)对就处于Active状态，`me_key`和`me_value`都不能为NULL且`me_key`不能时dummy对象；
  - entry中存储的(key,value)对被删除后，entry不能直接从Active状态转换为Unused状态，否则会导致冲突链的中断；将`me_key`和`me_value`指向dummy对象，entry进入Dummy状态；当沿某条冲突链搜索时发现一个entry处于Dummy状态说明该entry虽然无效但其后的entry可能有效，这样保证了冲突链的连续性；

PyDictObject定义如下：
```C++
#define PyDict_MINSIZE 8
class PyDictObject : public PyObject {
  Py_ssize_t ma_fill; // 元素个数：Active + Dummy
  Py_ssize_t ma_used; // 元素个数：Active
  Py_ssize_t ma_mask;
  PyDictEntry* ma_table;
  PyDictEntry* (*ma_lookup)(PyDictObject* mp, PyObject* key, long hash);
  PyDictEntry ma_smalltable[PyDict_MINSIZE];
};
```
当创建一个PyDictObject的时候至少有`PyDict_MINSIZE`个entry被同时创建，`me_table`指向可以作为PyDictEntry集合的内存的开始位置；当dict的entry数量少于8个时`ma_table`指向`ma_smalltable`，当超过8个时，会申请额外的内存空间，并将`ma_table`指向这块空间；这样`ma_table`永远不会为NULL；

PyDictObject初始化操作：
```C++
#define INIT_NONZERO_DICT_SLOTS(mp) do {\
  (mp)->ma_table = (mp)->ma_smalltable;\
  (mp)->ma_mask = PyDict_MINSIZE - 1;\
} while (0)

#define EMPTY_TO_MISIZE(mp) do {\
  memset((mp)->ma_smalltable, 0, sizeof((mp)->ma_smalltable));\
  (mp)->ma_used = (mp)->ma_fill = 0;\
  INIT_NONZERO_DICT_SLOTS(mp);\
} while (0)
```
第一次调用`PyDict_New`的时候创建的dummy对象是一个PyStringObject对象，dummy只是用来作为一种指示标志的；如果对象缓存池不可用则从系统申请合适的内存空间，然后通过两个宏完成初始化工作：
  - `EMPTY_TO_MISIZE`：将`ma_smalltable`清零，同时设置`ma_used`和`ma_fill`，在一个PyDictObject刚创建的时候，这两个变量都为0；
  - `INIT_NONZERO_DICT_SLOTS`：将`ma_table`指向`ma_smalltable`，将`ma_mask`设置为7；

Python有2种搜索策略，`lookdict/lookdict_string`，`lookdict_string`只是`lookdict`的一种针对PyStringObject的特殊形式；
  * 根据hash值获得entry的索引，这是冲突探测链上的第一个entry的索引
  * 下面2种情况，搜索结束：
    - entry处于Unused状态，冲突探测链搜索完成，搜索失败：
    - `ep->me_key == key`，表明entry的key与待搜索的可以匹配，搜索成功；
  * 若当前entry处于Dummy状态，设置freeslot；
  * 检测Active状态的entry中的key与待查找的key是否“值相等”，相等则搜索成功；
  * 根据Python所采用的探测函数，获取探测链下一个待检查的entry；
  * 检测到一个Unused状态的entry，搜索失败，有2中情况：
    - 如果freeslot不为空，则返回freeslot所指的entry；
    - 如果freeslot为空，则返回该Unused状态的entry；
  * 检查entry中的key与待查找的key是否符合“引用相同”规则；
  * 检查entry中的key与待查找的key是否符合“值相同”规则；
  * 在遍历过程中，发现Dummy状态的entry且freeslot未设置，则设置freeslot；

`lookdict_string`的搜索过程与`lookdict`类似，只是假设来需要搜索的key是一个PyStringObject对象；这里只对key进行了假设，没对参与搜索的dict做出假设，当参数搜索的dict中所有entry的key都是PyIntObject对象时都会采用`lookdict_string`进行搜索，`_PyString_Eq`将保证能处理非PyStringObject的参数；

dict的插入中在insertdict的操作，进行key搜索之后：
  * 搜索成功，返回处于Active状态的entry，直接替换`me_value`；
  * 搜索失败，返回Unused状态和Dummy状态的entry，完整设置`me_key`、`me_hash`和`me_value`；

insertdict在插入之后，当table的装载率大于2/3时，后续的插入遇到冲突的可能性会非常大，所以装载率是否大于或等于2/3就是判断是否需要改变table大小的准则，判断的算法如下：
```C++
if (!(mp->ma_used > n_used && mp->ma_fill*3 >= (mp->ma_mask+1)*2))
  return 0;

// 实际转换后为：
(mp->ma_fill) / (mp->ma_mask + 1) >= 2/ 3
```
在确定新table大小的时候，通常是现在table中Active状态entry数量的4倍；这样能让让处于Active状态的entry分布更稀疏，减少插入元素时的冲突概率；当table中Active状态的entry数量非常大（一般定为50000）时，只会要求2倍的空间；

删除一个元素的操作，先计算hash值再搜索对应的entry，最后删除entry中维护的元素，并将entry从Active状态转换为Dummy状态，同时调整维护的table使用情况的变量；
```C++
int PyDict_DelItem(PyObject* mp, PyObject* key) {
  // ...

  if (!PyString_CheckExact(key) ||
    (hash = ((PyStringObject*)key)->ob_shash) == -1) {
    hash = PyObject_Hash(key);
    if (hash == -1)
      return -1;
  }

  ep = (mp->ma_lookup)(mp, key, hash);
  if (ep->me_value == nullptr)
    return -1;

  old_key = ep->me_key;
  ep->me_key = dummy;
  old_value = ep->me_value;
  ep->me_value = nullptr;
  --mp->ma_used;
  Py_DECREF(old_key);
  Py_DECREF(old_value);
  return 0;
}
```

## **PyDictObject对象缓冲池**
dict的缓冲池相关:
```C++
#define MAXFREEDICTS 80
static PyDictObject* free_dicts[MAXFREEDICTS];
static int num_free_dicts = 0;
```
dict对象中使用的缓冲池机制与list中的一致，直到第一个dict对象销毁的时候缓冲池才开始接纳被缓冲的对象；缓冲池只保留PyDictObject独享，如果`ma_table`维护的是从系统堆上申请的内存空间，python将释放这块内存归还给系统堆；如果`ma_table`指向固有的`ma_smalltable`那么只需要调整`ma_smalltable`中的对象的引用计数即可；新创建dict对象的时候如果缓冲池中有可以使用的对象则直接从缓冲池中取；
