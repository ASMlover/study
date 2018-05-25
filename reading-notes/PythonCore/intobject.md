# **Python中的整数对象**
***

在Python中，整数对象运用很频繁，考虑Python采用的引用计数机制，频繁创建销毁必然会带来性能上的问题；而Python采用的解决方案就是【对象缓冲池】，几乎所有的Python内建对象都有自己特有的对象池；

整数对象的定义如下：
```C++
class PyIntObject : public PyObject {
  long ob_ival;
};
```

PyIntObject与对象相关的元信息保存在与对象对应的类型对象`PyInt_Type`中，PyIntObject所支持的主要操作集中在`int_as_number`中；

PyIntObject是一个immutable的对象，在操作完成之后，原来参与操作的任何一个对象都没有发生任何变化，而是返回一个全新的PyIntObject对象；如果操作（如加法、乘法……）结果有溢出，那就返回一个PyLongObject对象；

## **1、创建对象的3种途径**
创建PyIntObject的3种途径为：
```C++
PyObject* PyInt_FromLong(long ival);
PyObject* PyInt_FromString(char* s, char** pend, int base);
#ifdef PY_USING_UNICODE
PyObject* PyInt_FromUnicode(Py_UNICODE* s, int length, int base);
#endif
```
`PyInt_FromString`和`PyInt_FromUnicode`都先将字符串或`Py_UNICODE`对象转换成浮点数再调用`PyInt_FromLong`来创建int对象的；

## **2、整数对象存放**
小整数对象使用了对象池技术，PyIntObject是不可变对象，表示对象池中的每个PyIntObject对象都能被任意共享；在Python中小整数集合范围默认设定为[-5, 257)；

大整数对象存放在PyIntBlock结构指定的单向列表中：
```C++
struct PyIntBlock {
  PyIntBlock* next;
  PyIntObject objects[N_INTOBJECTS];
};

PyIntBlock* block_list = nullptr;
PyIntBlock* free_list = nullptr;
```
PyIntBlock的单向链表由`block_list`维护，每个block中都维护了一个PyIntObject数组——objects，用于存储被缓存的PyIntObject对象；`free_list`管理全部block的objects中所有的空闲内存；

首次调用`PyInt_FromLong`或所有block空闲内存都用完的时候，`free_list`为NULL，则会触发申请新的PyIntBlock；新申请的PyIntBlock和已有的`block_list`建立连接，`free_list`指向剩余的PyIntObject；

不同的PyIntObject对象的objects中的空闲内存块是被连接在一起的，形成一个单向链表，表头的指针就是`free_list`；而将这些释放的对象连接到一起的是类对象中的`tp_dealloc`：
```C++
static void int_dealloc(PyIntObject* v) {
  if (PyInt_CheckExact(v)) {
    v->ob_type = (sturct _typeobject*)free_list;
    free_list = v;
  }
  else {
    v->ob_type->tp_free((PyObject*)v);
  }
}
```
