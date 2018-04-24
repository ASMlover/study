# **Python内部对象**
***

## **对象基石——PyObject**
1、Python对象机制的核心PyObject
``` c
#define PyObject_HEAD\
  Py_ssize_t ob_refcnt;
  struct _typeobject* ob_type;

typedef struct _object {
  Py_ssize_t ob_refcnt;
  struct _typeobject* ob_type;
} PyObject;
```
每个对象除了必须包含这个PyObject的内容之外，还应该占有一些额外的内存来存放其他必要的信息；这里用C++来理解的话，就是PyObject是基类，其他类型对象都继承自PyObject；如下：
``` c++
class PyObject {
public:
  Py_ssize_t ob_refcnt;
  struct _typeobject* ob_type;
};

class PyIntObject : private PyObject {
  long ob_val;
};
```
