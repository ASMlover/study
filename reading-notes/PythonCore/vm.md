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
```

**`LOAD_CONST`**
```C++
x = GETITEM(consts, oparg);
Py_INCREF(x);
PUSH(x);
```
