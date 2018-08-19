# **虚拟机中的一般表达式**
***

常用的宏定义(方便遍历字节码指令):
```C++
// 访问tuple中的元素
#define GETITEM(v, i) PyTuple_GET_ITEM((PyTupleObject*)(v), (i))
// 调整栈顶指针
#define BASIC_STACKADJ(n) (stack_pointer += n)
#define STACKADJ(n) BASIC_STACKADJ(n)
// 入栈操作
#define BASIC_PUSH(v) (*stack_pointer++ = (v))
#define PUSH(v) BASIC_PUSH(v)
// 出栈操作
#define BASIC_POP() (*--stack_pointer)
#define POP() BASIC_POP()

#define TOP() (stack_pointer[-1])
#define SECOND() (stack_pointer[-2])
#define SET_TOP(v) (stack_pointer[-1] = (v))
#define SET_SECOND(v) (stack_pointer[-2] = (v))
```

**`LOAD_CONST`**
```C++
x = GETITEM(consts, oparg);
Py_INCREF(x);
PUSH(x);
```

**`STORE_NAME`**
通过执行字节码指令`STORE_NAME`来改变local名字空间
```C++
w = GETITEM(names, oparg); // 从符号表获得符号
v = POP(); // 从运行时栈获得值
if ((x = f->f_locals) != NULL) {
  // 将（符号、值）的映射关系存储到local名字空间中
  if (PyDict_CheckExact(x))
    PyDict_SetItem(x, w, v);
  else
    PyObject_SetItem(x, w, v);
  Py_DECREF(v);
}
```

**`BUILD_MAP`**
会在执行指令的时候创建一个空的PyDictObject独享，并把这个对象压入运行时栈中：
```C++
x = PyDict_New();
PUSH(x);
```

**`BUILD_LIST`**
与`BUILD_MAP`功能类似，但是它利用来这个指令的参数:
```C++
x = PyList_New(oparg);
if (x != nullptr) {
  for (; --oparg >= 0;) {
    w = POP();
    PyList_SET_ITEM(x, oparg, w);
  }
  PUSH(x);
}
```

**`STORE_SUBSCR`**
```C++
w = TOP();
v = SECOND();
u = THIRD();
STACKADJ(-3);
// v[w] = u, => dict['key'] = val
PyObject_SetItem(v, w, u);
Py_DECREF(u);
Py_DECREF(v);
Py_DECREF(w);
```
STACKADJ执行后，栈顶指针回退3格，所以`STORE_SUBSCR`指令执行完后，运行时栈里就只剩下最初由`BUILD_MAP`创建的PyDictObject对象；

**`LOAD_NAME`**
```C++
// 获得变量名
w = GETITEM(names, oparg);
// 在local名字空间中查找变量名对应的变量值
v = f->f_locals;
x = PyDict_GetItem(v, w);
Py_XINCREF(x);
if (x == nullptr) {
  // 在global名字空间中查找变量名对应的变量值
  x = PyDict_GetItem(f->f_globals, w);
  if (x == nullptr) {
    // 在builtin名字空间中查找变量名对应的变量值
    x = PyDict_GetItem(f->f_builtins, w);
    if (x == nullptr) {
      // 查找变量名失败，抛出异常
      format_exc_check_arg(PyExc_NameError, NAME_ERROR_MSG, w);
      break;
    }
  }
  Py_INCREF(x);
}
PUSH(x);
```
`LOAD_NAME`符合LGB的规则，先在locals名字空间中查找，再在globals名字空间中查找，最后再从builtins名字空间中查找；

**`BINARY_ADD`**
```C++
  w = POP();
  v = TOP();
  if (PyInt_CheckExact(v) && PyInt_CheckExact(w)) {
    // PyIntObject对象相加的快速通道
    atuo a = PyInt_AS_LONG(v);
    auto b = PyInt_AS_LONG(w);
    auto i = a = b;
    // 检查如果加法运算溢出，转向慢速通道进行计算
    if ((i ^ a) < 0 && (i ^ b) < 0)
      goto slow_add;
    x = PyInt_FromLong(i);
  }
  else if (PyString_CheckExact(v) && PyString_CheckExact(w)) {
    // str对象相加的快速通道
    x = string_concatenate(v, w, f, next_instr);
    goto skip_decref_vx;
  }
  else {
    // 一般对象相加的慢速通道
slow_add:
    x = PyNumber_Add(v, w);
  }
  Py_DECREF(v);
skip_decref_vx:
  Py_DECREF(w);
  SET_TOP(x);
```
计算方式：
  * 参与计算的两个对象是PyIntObject对象，会直接将PyIntObject的value提取出来相加，然后根据相加的结果创建新的PyIntObject对象作为结果返回；
  * 如果是PyStringObject对象之间相加，Python虚拟机会选择`string_concatenate`以加快速度；
  * 如果在这两种情况之外则只能走慢速通道`PyNumber_Add`完成运算；虚拟机会进行大量的类型判断，寻找与对象相对应的计算操作函数等额外工作；1）先检查参与运算的对象的类型对象，检查PyNumberMethods中的`nb_add`能否完成v和w上的加法运算；2）如果不能则会检查PySequenceMethods中的`sq_concat`能否完成，如果不能则只能报告错误；

**`PRINT_ITEM`**
```C++
v = POP(); // 获得要输出的对象
if (stream == nullpter || stream == Py_None)
  w = PySys_GetObject("stdout");
Py_XINCREF(w);
if (w != nullptr && PyFile_SoftSpace(w, 0))
  err = PyFile_WriteString(" ", w);
if (err == 0)
  err = PyFile_WriteObject(v, w, Py_PRINT_RAW);
...
stream = nullptr;
```
