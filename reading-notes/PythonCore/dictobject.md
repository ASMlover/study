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
