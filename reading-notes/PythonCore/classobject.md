# **对象模型**

Python2.2之前，有3类对象：
  * type对象：表示Python内置的类型；
  * class对象：表示Python程序员定义的类型；
  * instance对象（实例对象）：表示由class对象创建的实例；

Python的3中对象之间存在2种关系：
  * is-kind-of关系：对应面向对象中的基类与子类之间的关系；
  * is-instance-of关系：对应面向对象中类与实例之间的关系；

`<type 'type'>`是Python中特殊的class对象，它能称为其他class对象的type；这种特殊的class对象是metaclass对象；创建一个class对象的关键是metaclass对象；在Python中任何一个class都是必须间接或直接继承自object，object是万物之母；

Python中type总结：
  * 任何一个对象都有一个type，可以通过对象的__class__属性获得；任何一个instance对象的type都是一个class对象，任何一个class对象的type都是一个metaclass对象；一般这个metaclass都是`<type 'type'>`，在python内部为`PyType_Type`；
  * 任何一个class对象都直接或间接与`<type 'type'>`对象之间存在is-kind-of关系，包括`<type 'type'>`；在python内部`<type 'object'>`对应的是`PyBaseObject_Type`；

int对象的加法操作，在Python虚拟机需要调用`int.__add__`时，可以到符号int对应的class对象（`PyInt_Type`）的`tp_dict`指向的dict对象中查找符号`__add__`对应的操作，并调用该操作，从而完成`int.__add__`的调用；

只要一个对象对应的calss对象中实现了`__call__`操作（PyTypeObject中的`tp_call`不为空）那这个对象就是一个可调用对象；一个对象是否可调用并不在编译期能确定，必须在运行时才能在`PyObject_CallFunctionObjArgs`中确定；

Python启动的时候，会对类型（对象模型）进行初始化动作，这个初始化动作会动态地的内置类型对应的PyTypeObject中填充一些重要的数据（包括填充`tp_dict`），从而完成内置类型从type对象到class对象的转变（从`_Py_ReadyTypes`开始）；

处理基类和type信息
```c++
int PyType_Ready(PyTypeObject* type) {
  // 尝试获得type的tp_base中指定的基类（super type）
  PyTypeObject* base = type->tp_base;
  if (base == nullptr && type != &PyBaseObject_Type)
    base = type->tp_base = &PyBaseObject_Type;

  // 如果基类没有初始化，先初始化基类
  if (base && base->tp_dict == nullptr)
    PyType_Ready(base);

  // 设置type信息
  if (type->ob_type == nullptr && base != nullptr)
    type->ob_type = base->ob_type;
  ...
}
```
指定了`tp_base`的内置class对象，就使用指定的基类；没有指定`tp_base`的内置class对象，python将为其指定一个默认的基类(`PyBaseObject_Type`)；设置的`ob_type`就是metaclass，python虚拟机是将基类的metaclass作为子类的metaclass；对于`PyType_Type`而言其metaclass就是`<type 'object'>`的metaclass；

虚拟机处理基类列表，由于Python支持多重继承，所以每个Python的class对象会有一个基类列表：
```c++
int PyType_Ready(PyTypeObject* type) {
  ...

  // 尝试获得type的tp_base中指定基类（super typh）
  PyTypeObject* base = type->tp_base;
  if (base == nullptr && type != &PyBaseObject_Type)
    base = type->tp_base = &PyBaseObject_Type;

  ...
  // 处理bases，基类列表
  PyObject* bases = type->tp_bases;
  if (bases == nullptr) {
    // 如果bases为空，则根据base的情况设定bases
    if (base == nullptr)
      bases = PyTuple_New(0);
    else
      bases = PyTuple_Pack(1, base);
    type->tp_bases = bases;
  }
}
```

**填充`tp_dict`**
```c++
int PyType_Ready(PyTypeObject* type) {
  ...
  // 设定tp_dict
  PyObject* dict = type->tp_dict;
  if (dict == nullptr) {
    dict = PyDict_New();
    type->tp_dict = dict;
  }

  // 将与type相关的descriptor加入到tp_dict中
  add_operators(type);
  if (type->tp_methods != nullptr)
    add_methods(type, type->tp_methods);
  if (type->tp_members != nullptr)
    add_members(type, type->tp_members);
  if (type->tp_getset != nullptr)
    add_getset(type, type->tp_getset);
  ...
}
```
这里完成将`("__add__", &nb_add)`加入到`tp_dict`的过程，这个阶段的`add_operators`、`add_methods`、`add_members`、`add_getset`都是完成这样的填充`tp_dict`的动作；

slot在Python内部是表示PyTypeObject中定义的操作，一个操作对应一个slot，slot不仅包含一个函数指针，还包含其他一些信息，slot是通过slotdef这个结构体来实现；
```c++
// [typeobject.c]
typedef struct wrapperbase slotdef;

// [descrobject.h]
struct wrapperbase {
  char* name;
  int offset;
  void* function;
  wrapperbase wrapper;
  char* doc;
  int flags;
  PyObject* name_strobj;
};
```
在一个slot中存储着与PyTypeObject中一种操作相对应的各种信息；offset则是操作的函数地址在PyHeapTypeObject中的偏移量；function指向一种slot function的函数；由于object的member function在`tp_as_number`等的指针上，没法极端在PyTypeObject中的偏移量，只能计算其在PyHeapTypeObject中的偏移量；
```c++
typedef struct _heaptypeobject {
  PyTypeObject ht_type;
  PyNumberMethods as_number;
  PyMappingMethods as_mapping;
  PySequenceMethods as_sequence;
  PyBufferProcs as_buffer;
  PyObject* ht_name;
  PyObject* ht_slots;
} PyHeapTypeObject;
```
对slotdefs的排序在`init_slotdefs`中完成的：
```c++
static void init_slotdefs(void) {
  static bool initialized{};
  // init_slotdefs只会进行一次
  if (initialized)
    return;
  for (auto* p = slotdefs; p->name; p++) {
    // 填充slotdef结构体中的name_strobj
    p->name_strobj = PyString_InternFromString(p->name);
  }
  // 对slotdefs中的slotdef进行排序
  qsort((void*)slotdefs, (size_t)(p-slotdefs), sizeof(slotdef), slotdef_cmp);
  initialized = true;
}

// slot排序的比较策略
static int slotdef_cmp(const void* aa, const void* bb) {
  const slotdef* a = (const slotdef*)aa;
  const slotdef* b = (const slotdef*)bb;
  if (c != nullptr)
    return c;
  else
    return (a > b) ? 1 : (a < b) ? -1 : 0;
}
```

`tp_dict`中与`__getitem__`对应的是一个包装了slot的PyObject，这就是descriptor；与PyTypeObject中的操作对应的是PyWrapperDescrObject；一个descriptor包含一个slot，其创建是通过`PyDescr_NewWrapper`完成的：
```c++
#define PyDescr_COMMON\
  PyObject_HEAD\
  PyTypeObject* d_type;\
  PyObject* d_name

typedef struct {
  PyDescr_COMMON;
  struct wrapperbase* d_base;
  void* d_wrapped; // this can be any function pointer
} PyWrapperDescrObject;

static PyDescrObject*
descr_new(PyTypeObject* descrtype, PyTypeObject* type, char* name) {
  auto* descr = (PyDescrObject*)PyType_GenericAlloc(descrtype, 0);
  descr->d_type = type;
  descr->d_name = PyString_InternFromString(name);
  return descr;
}

PyObject*
PyDescr_NewWrapper(
  PyTypeObject* type, struct wrapperbase* base, void* wrapped) {
  PyWrapperDescrObject* descr = descr_new(
      &PyWrapperDescr_Type, type, base->name);
  descr->d_base = base;
  descr->d_wrapped = wrapped;
  return (PyObject*)descr;
}
```
Python内部各种descriptor都包含`PyDescr_COMMON`，`d_type`被设置为`PyDescr_NewWrapper`的参数type，`d_wrapped`存放着重要的信息（操作对应的函数指针，对list来说`tp_dict["__getitem__"].d_wrapped`对应的就是`&mp_subscript`）；slot存放在`d_base`中；

排序后的结果存在slotdefs中，虚拟机可以从头到尾遍历slotdefs，基于每个slot建立一个descriptor，然后在`tp_dict`中建立从操作名到descriptor的关联；这个过程在`add_operators`中完成：
```c++
static int add_operators(PyTypeObject* type) {
  auto* dict = type->tp_dict;
  PyObject* descr;
  void** ptr;

  // 对slotdefs进行排序
  init_slotdefs();
  for (auto* p = slotdefs; p->name; p++) {
    // 如果slot中没有指定wrapper，则不处理
    if (p->wrapper == nullptr)
      continue;
    // 获得slot对应的操作在PyTypeObject中的函数指针
    ptr = slotptr(type, p->offset);
    // 如果tp_dict中已经存在操作名，则放弃
    if (PyDict_GetItem(dict, p->name_strobj))
      continue;
    // 创建descriptor
    descr = PyDescr_NewWrapper(type, p, *ptr);
    // 将（操作名，descriptor）放入tp_dict中
    PyDict_SetItem(dict, p->name_strobj, descr);
  }
  return 0;
}
```
