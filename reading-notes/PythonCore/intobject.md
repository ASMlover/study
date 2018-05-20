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

## **2、小整数对象**
小整数对象使用了对象池技术，PyIntObject是不可变对象，表示对象池中的每个PyIntObject对象都能被任意共享；在Python中小整数集合范围默认设定为[-5, 257)；
