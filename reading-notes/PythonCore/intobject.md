# **Python中的整数对象**
***

在Python中，整数对象运用很频繁，考虑Python采用的引用计数机制，频繁创建销毁必然会带来性能上的问题；而Python采用的解决方案就是【对象缓冲池】，几乎所有的Python内建对象都有自己特有的对象池；

整数对象的定义如下：
```c++
class PyIntObject : public PyObject {
  long ob_ival;
};
```

PyIntObject与对象相关的元信息保存在与对象对应的类型对象`PyInt_Type`中，PyIntObject所支持的主要操作集中在`int_as_number`中；
