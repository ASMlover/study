# **虚拟机的函数机制**
***

```C++
class PyFunctionObject : public PyObject {
  PyObject* func_code; // 对应函数编译后的PyCodeObject对象
  PyObject* func_globals; // 函数运行时的global名字空间
  PyObject* func_defaults; // 默认参数(tuple或nullptr)
  PyObject* func_closure; // nullptr或tuple of cell objects，用于实现closure
  PyObject* func_doc; // 函数的文档
  PyObject* func_name; // 函数名称，函数的__name__属性
  PyObject* func_dict; // 函数的__dict__属性
  PyObject* func_weakreflist;
  PyObject* func_module; // 函数的__module__可以是任意对象
};
```
PyCodeObject和PyFunctionObject都和函数相关：PyCodeObject是对一段Python源代码的静态表示，源代码经过编译后，对一个Code Block会产生一个且只有一个PyCodeObject，其中包含来这个Code Block的一些静态信息，是指可从源代码中看到的信息；PyFunctionObject是Python代码在运行时动态产生的，是执行一个def语句时创建的，PyFunctionObject中包含的这个函数的静态信息存储在`func_code`（PyCodeObject）中；

对一段Python代码，对应的PyCodeObject只有一个，而代码所对应的PyFunctionObject对象却可能有多个；一个函数多次调用，则会在运行时创建多个PyFunctionObject对象，每个PyFunctionObject的`func_code`都会关联这个PyCodeObject对象；
