# **Python编译结果——Code对象与pyc文件**
***

## **1、编译**
编译中，包含在源代码中的静态信息会被编译器收集起来，编译结果包含字符串、常量值、字节码等在源代码中出现的一切有用的静态信息；

python运行期，源文件中提供的静态信息最终会被存储在一个运行时对象；运行结束后该运行时对象中所包含的信息会被存储在pyc文件中（这个运行时对象就是PyCodeObject）；

程序运行期间，编译结果存在内存的PyCodeObject对象中；运行结束后，编译结果则被保存在pyc文件中；下一次运行的时候会根据pyc中记录的编译结果直接在内存中建立PyCodeObject对象；

## **PyCodeObject**
```C++
class PyCodeObject : public PyObject {
  int co_argcount;
  int co_nlocals;
  int co_stacksize;
  int co_flags;
  PyObject* co_code; // 存放编译所生成的字节码指令序列
  PyObject* co_consts;
  PyObject* co_names;
  PyObject* co_varnames;
  PyObject* co_freevars;
  PyObject* co_cellvars;
  PyObject* co_filename;
  PyObject* co_name;
  int co_firstlineno;
  PyObject* co_lnotab;
  void* co_zombieframe;
};
```
对于代码中的一个Code Block，会有一个PyCodeObject与之对应；当进入一个新的名字空间的时候，或进入一个新的作用域的时候就进入了一个新的Code Block；

在一个名字空间中一个符号只能有一种意义，名字空间可以一个套一个形成一条名字空间链；Code Block对应一个名字空间，对应一个PyCodeObject对象；在Python中类、函数、module都对应一个独立的名字空间；

如在`import abc`这样语句中，python会到设定好的path中寻找abc.pyc或abc.so(abc.dll)，如果没有这些文件而只有abc.py的时候会先编译出对应的PyCodeObject的中间结果，然后创建abc.pyc并将中间结果写入该文件；

`co_argcount`: Code Block的位置参数的个数（比如一个函数的位置参数个数）
`co_nlocaks`: Code Block中局部变量的个数，包括其位置参数的个数
`co_stacksize`: 执行该段Code Block需要的栈空间
`co_flags`: N/A
`co_code`: Code Block编译所得到的字节码指令序列，以PyStringObject的形式存在
`co_consts`: PyTupleObject对象，保存Code Block中的所有常量
`co_names`: PyTupleObject对象，保存Code Block中的所有符号
`co_varnames`: Code Block中的局部变量名集合
`co_freevars`: Python实现闭包需要用到的东西
`co_cellvars`: Code Block中内部嵌套函数所引用的局部变量名集合
`co_filename`: Code Block所对应的.py文件的完整路径
`co_name`: Code Block名字，通常是函数名或者类名
`co_firstlineno`: Code Block在对应.py文件中的起始行
`co_lnotab`: 字节码指令与.py文件中source code行号的对应关系，以PyStringObject的形式存在
