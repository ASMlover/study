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
