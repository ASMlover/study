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

Python启动的时候，会对类型（对象模型）进行初始化动作，这个初始化动作会动态地的内置类型对应的PyTypeObejct中填充一些重要的数据（包括填充`tp_dict`），从而完成内置类型从type对象到class对象的转变（从`_Py_ReadyTypes`开始）；
