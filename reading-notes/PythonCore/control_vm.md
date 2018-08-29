# **虚拟机中的控制流**
***

经过Python编译器编译后的字节码序列结构：
  * 执行`LOAD_NAME`指令，从local名字空间获得变量名对应的变量值；
  * 执行`LOAD_CONST`指令，从常量表consts中读取参与该分支判断操作的常量对象；
  * 执行`COMPARE_OP`指令，对前面两条指令取得的变量值和常量对象进行比较操作；
  * 指向某`JUMP_*`指令，根据`COMPARE_OP`指令的运行结果进行字节码指令跳跃;

**`COMPARE_OP`**
```C++
  w = POP();
  v = TOP();
  // PyIntObject的快速通道处理
  if (PyInt_CheckExact(w) && PyInt_CheckExact(v)) {
    register int res;
    auto a = PyInt_AS_LONG(v);
    auto b = PyInt_AS_LONG(w);
    // 根据字节码指令的指令参数选择不同的比较操作
    switch (oparg) {
    case PyCmp_LT: res = a < b; break;
    case PyCmp_LE: res = a <= b; break;
    case PyCmp_EQ: res = a == b; break;
    case PyCmp_NE: res = a != b; break;
    case PyCmp_GT: res = a > b; break;
    case PyCmp_GE: res = a >= b; break;
    case PyCmp_IS: res = v == w; break;
    case PyCmp_IS_NOT: res = v != w; break;
    default: goto slow_compare;
    }
    x = res ? Py_True : Py_False;
    Py_INCREF(x);
  }
  else {
    // 一般对象的慢速通道
slow_compare:
    x = cmp_outcome(oparg, v, w);
  }
  Py_DECREF(v);
  Py_DECREF(w);
  // 将比较结果压入到运行时栈中
  SET_TOP(x);
  if (x == nullptr)
    break;
  PREDICT(JUMP_IF_FALSE);
  PREDICT(JUMP_IF_TRUE);

///////////////////////////////////////////////
// ceval.c
static PyObject* cmp_outcome(int op, PyObject* v, PyObject* w) {
  int res = 0;
  switch (op) {
  case PyCmp_IS: res = (v == w); break;
  case PyCmp_IS_NOT: res = (v != w); break;
  case PyCmp_NOT_IN:
    res = PySequence_Contains(w, v);
    if (res < 0)
      return nullptr;
    res = !res;
    break;
  case PyCmp_EXC_MATCH: res = PyErr_GivenExceptionMatches(v, w); break;
  default: return PyObject_RichCompare(v, w, op);
  }
  v = res ? Py_True : Py_False;
  Py_INCREF(v);
  return v;
}
```

在python中bool对象是PyObject对象：
```C++
// boolobject.h
// do't ues these directly
PyIntObject _Py_ZeroStruct, _Py_TrueStruct;

// use these macros
#define Py_False ((PyObject*)&_Py_ZeroStruct)
#define Py_True ((PyObject*)&_Py_TrueStruct)

// boolobject.c
// the type object for bool, note that this cannot be subclassed
PyTypeObject PyBool_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,
  "bool",
  sizeof(PyIntObject),
  ...
};

PyIntObject _Py_ZeroStruct = {
  PyObject_HEAD_INIT(&PyBool_Type)
  0
};

PyIntObject _Py_TrueStruct = {
  PyObject_HEAD_INIT(&PyBool_Type)
  1
};
```

**PREDICT**

虚拟机中字节码指令跳跃的实现在PREDICT宏：
```C++
// ceval.c
#define PREDICT(op) if (*next_instr == op) goto PRED_##op
#define PREDICTED(op) PRED_##op: next_instr++
#define PREDICTED_WITH_ARG(op) PRED_##op: oparg = PEEKARG(); next_instr += 3
#define PEEKARG() ((next_instr[2] << 8) + next_instr[1])

#define JUMPBY(x) (next_instr += (x))
```
`PREDICT(JUMP_IF_FALSE)`实际检查下一条待处理的字节码是否是`JUMP_IF_FALSE`，如果是，则程序流程跳转到`PRED_JUMP_IF_FALSE`标识符对应的代码处；

**`SETUP_LOOP`**
```C++
case SETUP_LOOP:
case SETUP_EXCEPT:
case SETUP_FINALLY:
  PyFrame_BlockStep(f, opcode, INSTR_OFFSET() + oparg, STACK_LEVEL());

// in frameobject.c
void PyFrame_BlockStep(PyFrameObject* f, int type, int handler, int level) {
  auto* b = &f->f_blockstack[f->f_iblock++];
  b->b_type = type;
  b->b_level = level;
  b->b_handler = handler;
}
```

**PyTryBlock**

`f_blockstack`的结构如下：
```C++
#define CO_MAXBLOCKS 20

typedef struct _frame {
  ...
  int f_iblock; // index in f_blockstack
  PyTryBlock f_blockstack[CO_MAXBLOCKS]; // for try and loop blocks
} PyFrameObject;

typedef srtuct {
  int b_type; // what kind of block this is
  int b_handler; // where to jump to find handler
  int b_level; // value stack level to pop to
} PyTryBlock;
```
`SETUP_LOOP`指令所做的就是从`f_blockstack`这个数组中获取一块PyTryBlock结构并存放一些Python虚拟机当前的状态信息；

**`GET_ITER`**
```C++
// 从运行时栈获得PyListObject对象
v = TOP();
// 获得PyListObject对象的iterator
x = PyObject_GetIter(v);
Py_DECREF(v);
if (x != nullptr) {
  // 将PyListObject对象的iterator压入堆栈
  SET_TOP(x);
  PREDICT(FOR_ITER);
  continue;
}
STACKADJ(-1);
```
虽然Python中list对象在内存布局上与C++的vector相同，但是其对应的迭代器却和STL的list一致，只要拥有迭代器对象，这些迭代器都是一个实实在在的对象：
```C++
// in listobject.c
class listiterobject : public PyObject {
public:
  long it_index;
  PyListObject* it_seq; // set to nullptr, when iterator is exhasusted
};

// 迭代器listiterobject对象所对应的类型对象就是PyListIter_Type
PyTypeObject PyListIter_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  0, // ob_size
  "listiterator", // tp_name
  sizeof(listiterobject), // tp_basicsize
  0, // tp_itemsize
  // methods
  ...
  PyObject_SelfIter, // tp_iter
  (iternextfunc)listiter_next, // tp_iternext
  0, // tp_methods
  ...
};
```
PyListObject对象的迭代器只是对PyListObject对象做来一个简单的包装，在迭代器中维护了当前访问的元素在PyListObject对象中的序号`it_index`，这样就可以实现对PyListObject的遍历；在`GET_ITER`指令完成之后会开始对`FOR_ITER`指令的预测动作，这样是为了提高执行效率；

**`FOR_ITER`**

在`GET_ITER`指令之后，虚拟机应该进入一个与源代码对应的循环控制结构
```C++
PREDICTED_WITH_ARG(FOR_ITER);
case FOR_ITER:
  // 从运行时栈的栈顶获得iterator对象
  v = TOP();
  // 通过iterator对象获得集合中下一个元素对象
  x = (*v->ob_type->tp_iternext)(v);
  if (x != nullptr) {
    // 将获得的元素压入运行时栈
    PUSH(x);
    PREDICTED(STORE_FAST);
    PREDICT(UNPACK_SEQUENCE);
    continue;
  }
  // iterator ended normally, x == nullptr标识iterator的迭代已经结束
  x = v = POP();
  Py_DECREF(v);
  JUMPBY(oparg);
  continue;
```

**`JUMP_ABSOLUTE`**
```C++
#define JUMPTO(x) (next_instr = first_instr + (x))

JUMPTO(oparg);
```
`JUMP_ABSOLUTE`指令的行为是强制设定`next_instr`的值，将IP设定到距离`f->f_code->co_code`开始地址的某一特定偏移的位置；
