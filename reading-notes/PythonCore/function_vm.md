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

函数的声明和函数的实现是分离的，分离在不同的PyCodeObject对象中；在能调用一个函数之前，Python必须先创建这个函数对象，这个对象是由`def f()`语句创建的，在虚拟机角度，它是函数对象的创建语句；虚拟机执行def语句的时候，会动态创建一个函数（一个PyFunctionObject对象）

**`MAKE_FUNCTION`**
```C++
  v = POP(); // 获得与函数f对应的PyCodeObject对象
  x = PyFunction_New(v, f->f_globals);
  Py_DECREF(v);
  ... // 处理函数参数的默认值
  PUSH(x);
  break;
```
`MAKE_FUNCTION`执行时，会将PyCodeObject对象弹出运行时栈，然后以该对象和当前PyFrameObject对象中威化的globals名字空间`f_globals`对象为参数，通过`PyFunction_New`创建一个新的PyFunctionObject对象，这个`f_globals`将称为函数在运行时的globals名字空间；

**`CALL_FUNCTION`**
```C++
  PyObject** sp = stack_pointer;
  x = call_function(&sp, oparg);
  stack_pointer = sp;
  PUSH(x);
  if (x != nullptr)
    continue;
  break;
```
function、cfunction和method的调用也会进入这个`call_function`；

在Python中，函数的参数类型可分为：
  * 位置参数（positional argument）；f(a, b)、a和b为位置参数；
  * 键参数（key argument）：f(a, b, name='Python')，其中name='Python'为键参数；
  * 扩展位置参数（excess positional argument）：f(a, b, *args)，其中*args为扩展位置参数；
  * 扩展键参数（excess key argument）：f(a, b, **kwds)，其中**kwds为扩展键参数；
```C++
static PyObject* call_function(PyObject** pp_stack, int oparg) {
  // 处理函数参数信息
  int na = oparg & 0xff;
  int nk = (oparg>>8) & 0xff;
  int n = na + 2 * nk;
  // 获得PyFunctionObject对象
  PyObject** pfunc = (*pp_stack) - n - 1;
  PyObject* func = *pfunc;
  ...
}
```
扩展位置参数在Python内部作为一个PyListObject对待的，扩展键参数在Python内部作为一个PyDictObject对待；
