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

**`LOAD_FAST` & `STORE_FAST`**
```C++
#define GETLOCAL(i) (fastlocals[i])
#define SETLOCAL(i, value) do {\
  PyObject* tmp = GETLOCAL(i);\
  GETLOCAL(i) = value;\
  Py_XDECREF(tmp);\
} while (0)

// [LOAD_FAST]
  x = GETLOCAL(oparg);
  if (x != nullptr) {
    Py_INCREF(x);
    PUSH(x);
    goto fast_next_opcode;
  }

// [STORE_FAST]
  v = POP();
  SETLOCAL(oparg, v);
  goto fast_next_opcode;
```

无论函数是否有参数，def语句编译后的结果都是一样的，差别在进行函数调用的时候产生，无参函数在调用前仅将PyFunctionObject对象压入运行时栈，带参函数还需要将参数也压入运行时栈；
```C++
// [MAKE_FUNCTION]
  // 获得PyCodeObject对象，并创建PyFunctionObject
  v = POP();
  x = PyFunction_New(v, f->f_globals);
  Py_DECREF(v);
  // 处理带默认值的函数参数
  if (x != nullptr && oparg > 0) {
    v = PyTuple_New(oparg);
    while (--oparg >= 0) {
      w = POP();
      PyTuple_SET_ITEM(v, oparg, w);
    }
    err = PyFunction_SetDefaults(x, v);
    Py_DECREF(v);
  }
  PUSH(x);
```
`MAKE_FUNCTION`指令的参数表示当前运行时栈中有几个函数参数的默认值，该指令会将指令参数指定的所有函数参数的默认值从运行时栈中弹出，放到一个PyTupleObject对象中，然后调用`PyFunction_SetDefaults`将该对象设置为`PyFunctionObject.func_defaults`的值，这样就称为PyFunctionObject的一部分；
```C++
// funcobject.c
int PyFunction_SetDefaults(PyObject* op, PyObject* defaults) {
  ((PyFunctionObject*)op)->func_defaults = defaults;
  return 0;
}

// in fast_function in ceval.c
static PyObject*
fast_function(PyObject* func, PyObject*** pp_stack, int n, int na, int nk) {
  PyCodeObject* co = (PyCodeObject*)PyFunction_GET_CODE(func);
  PyObject* globals = PyFunction_GET_GLOBALS(func);
  // 获得函数对应的PyFunctionObject中的func_defaults
  PyObject* argdefs = PyFunction_GET_DEFAULTS(func);
  PyObject** d{};
  int nd{};

  // 判断是否进入快速通道，argdefs != nullptr导致判断失败
  if (argdefs == nullptr && co->co_argcount == n && nk == 0 &&
      co->co_flags == (CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE)) {
    ...
  }

  // 获得函数参数的默认值信息（1.第一个默认参数的地址，2.默认值的个数）
  if (argdefs != nullptr) {
    d = &PyTuple_GET_ITEM(argdefs, 0);
    nd = ((PyTupleObject*)argdefs)->ob_size;
  }
  return PyEval_EvalCodeEx(co, globals, (PyObject*)nullptr,
            (*pp_stack)-n, na, // 位置参数的信息
            (*pp_stack)-2*nk, nk, // 键参数的信息
            d, nd, // 函数默认参数的信息
            PyFunction_GET_CLOSURE(func));
}
```
**默认位置参数**是指定了默认值的位置参数，没有指定默认值的则是**一般位置参数**；当调用函数传递的位置参数的个数小于函数编译后的PyCodeObject对象中的`co_argcount`指定的参数个数则说明Python虚拟机需要为函数设定默认参数；函数参数的默认值从函数参数列表的最右端开始，必须连续设置；

在PyCodeObject中，与嵌套函数相关的属性是`co_cellvars`和`co_freevars`，具体含义是：
  * `co_cellvars`：通常是一个tuple，保存嵌套的作用域中使用的变量名集合；
  * `co_freevars`：通常是一个tuple，保存使用了的外层作用域中的变量名集合；

当PyCodeObject的`co_cellvars`中有东西，在`PyEval_EvalCodeEx`中，Python虚拟机会同处理默认参数一样将`co_cellvars`中的东西拷贝到新创建的PyFrameObject的`f_localplus`中：
```C++
PyObject* PyEval_EvalCodeEx(...) {
  ...
  if (PyTuple_GET_SIZE(co->co_cellvars)) {
    bool found{}; // 标识被内层嵌套函数引用的符号是否已经与某个值绑定的标识，
                  // 与某个对象建立约束关系，只有在内层嵌套函数引用的是外层
                  // 函数的一个有默认值的参数时这个标识才能为True
    char* cellname;
    char* argname;
    PyObject* c;
    ...
    for (int i = 0; i < PyTuple_GET_SIZE(co->co_cellvars); ++i) {
      // 获得被嵌套函数共享的符号名
      cellname = PyString_AS_STRING(PyTuple_GET_ITEM(co->co_cellvars, i));
      found = false;
      ... // 处理被嵌套共享外层函数的默认参数
      if (!found) {
        c = PyCell_New(nullptr);
        if (c == nullptr)
          goto fail;
        SETLOCAL(co->co_nlocals + i, c);
      }
    }
  }
}
```

**PyCellObject**
```C++
class PyCellObject : public PyObject {
  PyObject* ob_ref; // content of the cell or nullptr when empty
};

// in cellobject.c
PyObject* PyCell_New(PyObject* obj) {
  auto* op = (PyCellObject*)PyObject_GC_New(PyCodeObject, &PyCell_Type);
  op->ob_ref = obj;
  Py_XINCREF(obj);
  _PyObject_GC_TRACK(op);
  return (PyObject*)op;
}
```

**`STORE_DEREF`**
```C++
// [PyEval_EvalFrameEx]
freevars = f->f_localplus + co->co_nlocals;

// [STORE_DEREF]
  w = POP();
  x = freevars[oparg];
  PyCell_Set(x, w);
  Py_DECREF(w);

// cellobject.h
#define PyCell_SET(op, v) (((PyCellObject*)(op))->ob_ref = v)

// cellobject.c
int PyCell_Set(PyObject* op, PyObject* obj) {
  Py_XDECREF(((PyCellObject*)op)->ob_ref);
  Py_XINCREF(obj);
  PyCell_SET(op, obj);
  return 0;
}
```

**`LOAD_CLOSURE`**
```C++
  x = freevars[oparg];
  Py_INCREF(x);
  PUSH(x);
```

**`MAKE_CLOSURE`**
```C++
  {
    v = POP(); // 获得PyCodeObject对象
    x = PyFunction_New(v, f->f_globals); // 绑定global名字空间
    v = POP(); // 获得tuple，其中包含PyCellObject对象的集合
    err = PyFunction_SetClosur(x, v); // 绑定约束集合
    ... // 处理拥有默认值的参数
    PUSH(x);
  }
```

`inner_func`对应的PyCodeObject中的`co_freevars`里有引用的外层作用域中符号名，在`PyEval_EvalCodeEx`就会对这个`co_freevars`进行处理：
```c++
// closure变量作为一个参数传递到PyEval_EvalCodeEx之中
// [funcobject.h]
#define PyFunction_GET_CLOSURE(func) (((PyFunctionObject*)func)->func_closure)

// [ceval.c]
PyObject* fast_function(...) {
  ...
  return PyEval_EvalCodeEx(..., PyFunction_GET_CLOSURE(func));
}

PyObject* PyEval_EvalCodeEx(...) {
  ...
  if (PyTuple_GET_SIZE(co->co_freevars)) {
    for (int i = 0; i < PyTuple_GET_SIZE(co->co_freevars); ++i) {
      auto* o = PyTuple_GET_ITEM(closure, i);
      freevars[PyTuple_GET_SIZE(co->co_cellvars) + i] = o;
    }
  }
  ...
}
```

**`LOAD_DEREF`**
```c++
  x = freevars[oparg]; // 获得PyCellObject对象
  w = PyCell_Get(x); // 获得PyCodeObject.ob_obj指向的对象
  if (w != nullptr) {
    PUSH(w);
    continue;
  }
  ...
```
