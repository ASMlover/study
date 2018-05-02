# **Python内部对象**
***

## **对象基石——PyObject**
1、Python对象机制的核心**PyObject**
``` c
#define PyObject_HEAD\
  int ob_refcnt;
  struct _typeobject* ob_type;

typedef struct _object {
  int ob_refcnt;
  struct _typeobject* ob_type;
} PyObject;
```
每个对象除了必须包含这个PyObject的内容之外，还应该占有一些额外的内存来存放其他必要的信息；这里用C++来理解的话，就是PyObject是基类，其他类型对象都继承自PyObject；如下：
``` c++
class PyObject {
public:
  int ob_refcnt;
  struct _typeobject* ob_type;
};

class PyIntObject : private PyObject {
  long ob_val;
};
```

2、定长对象和变长对象（PyVarObject）

对于一般定长的对象继承自PyObject即可（如int，bool等），但是对于变长的对象（如str，list，dict等）需要一个额外的信息来表示对象中容纳了多少个元素；这就是PyVarObject：
``` c++
class PyVarObject : public PyObject {
public:
  int ob_size;
};
```
有了PyVarObject我们定义变长对象就变得简单了，如：
``` c++
class PyStringObject : public PyVarObject {
  long ob_shash;
  int ob_sstate;
  char ob_sval[1];
};
```
在Python内部，每个对象都拥有相同的对象头部（PyObject），对对象的引用变得统一，只需要使用一个PyObject*就可以引用任意对象，而不用关心其具体时什么类型的对象。
