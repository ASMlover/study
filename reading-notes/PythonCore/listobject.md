# **PyListObject**
***

PyListObject是一个可变对象可在运行时动态调整其维护的内存和元素，其定义如下：
```C++
class PyListObject : public PyVarObject {
  using PyObjectVector = PyObject**;

  PyObjectVector ob_item; // 指向元素列表所在内存块首地址
  int allocated; // 当前列表中可容纳的元素总和
};
```
为了避免平凡申请内存，PyListObject在每一次申请内存时总会申请一大块内存，申请的总内存大小记录在allocated中，实际使用的内存存放在`ob_size`中；对于list对象存在：
```python
0 <= ob_size <= allocated
len(list) == ob_size
ob_item == NULL => ob_size == allocated == 0
```

## **1、创建PyListObject对象**
list对象表现为PyListObject对象本身和所维护的元素列表，是分离的内存，通过`ob_item`建立联系；创建list对象时会先查看缓存池`free_lists`中是否有可用的对象，如有则直接使用，没有才通过`PyObject_GC_New`申请内存创建新的PyListObject对象；默认情况下`free_lists`最多维护80个PyListObject对象；
```C++
#define MAXFREELISTS 80
static PyListObject* free_lists[MAXFREELISTS];
statit int num_free_lists = 0;
```
根据传递进`PyList_New`的size参数创建PyListObject对象所维护的元素列表，每个元素初始化为NULL；同时调整`ob_size`和`allocated`；

设置元素的时候根据索引校验索引的有效性，通过后将要加入的`PyObject*`指针放到指定为止，然后调整计数，将这个位置原来存放的对象引用计数减1；
