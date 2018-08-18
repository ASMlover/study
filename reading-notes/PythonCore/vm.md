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
