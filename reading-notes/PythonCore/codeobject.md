# **Python编译结果——Code对象与pyc文件**
***

## **编译**
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

PyCodeObject对象成员:
  * `co_argcount`: Code Block的位置参数的个数（比如一个函数的位置参数个数）
  * `co_nlocaks`: Code Block中局部变量的个数，包括其位置参数的个数
  * `co_stacksize`: 执行该段Code Block需要的栈空间
  * `co_flags`: N/A
  * `co_code`: Code Block编译所得到的字节码指令序列，以PyStringObject的形式存在
  * `co_consts`: PyTupleObject对象，保存Code Block中的所有常量
  * `co_names`: PyTupleObject对象，保存Code Block中的所有符号
  * `co_varnames`: Code Block中的局部变量名集合
  * `co_freevars`: Python实现闭包需要用到的东西
  * `co_cellvars`: Code Block中内部嵌套函数所引用的局部变量名集合
  * `co_filename`: Code Block所对应的.py文件的完整路径
  * `co_name`: Code Block名字，通常是函数名或者类名
  * `co_firstlineno`: Code Block在对应.py文件中的起始行
  * `co_lnotab`: 字节码指令与.py文件中source code行号的对应关系，以PyStringObject的形式存在

Python在import对module进行动态加载的时候，如果没有找到对应的pyc或dll文件就会在py文件的基础上自动创建pyc文件；
```c++
static void write_compiled_module(...) {
  ...
  fp = open_exclusive(pathname); // 排他性打开文件
  PyMarshal_WriteLongToFile(pyc_magic, fp, Py_MARSHAL_VERSION); // 写入magic number
  PyMarshal_WriteLongToFile(mtime, fp, Py_MARSHAL_VERSION); // 写入时间信息
  PyMarshal_WriteObjectToFile(o, fp, Py_MARSHAL_VERSION); // 写入PyCodeObject对象
  ...
}
```

一个pyc文件实际包含了3个独立信息：Python的magic number，pyc文件的创建时间信息，PyCodeObject对象
  * 不同版本的Python会有不同的magic number，用来保证Python的兼容性；因为不同版本的python存在字节码指令不一样的情况；
  * 在pyc中包含时间信息可以使Python自动将pyc文件与最新的py文件进行同步（当import的时候，加载pyc过程中，发现pyc文件的时间早于py文件的时间，会自动重新编译py文件，重新生存pyc）；
  * 最后将PyCodeObject对象写入pyc文件；

pyc写入的时候记录的WFILE.strings是一个dict，这是为了在二次记录相同str对象的时候直接记录写入的序号，而在读入pyc的时候WFILE.stirngs是个list，这样记录为`TYPE_STRINGREF`的字符串可以直接根据需要构建对应的字符串；对于一个intern字符串，Python（WFILE的strings）会查找其中是否已经记录来该字符串，导致两种情况：
  * 查找失败，进入intern字符串的首次写入，会进行2个独立动作：
    - 将（字符串、序号）添加到strings中
    - 将类型标识`TYPE_INTERN`和字符串本身写入pyc文件
  * 查找成功，进入intern字符串的非首次写入，仅只将类型标识`TYPE_STRINGREF`和查找的需要写入pyc

在对一个PyCodeObject进行写入pyc操作的时候，如果它包含另一个PyCodeObject对象，则继续递归进行写入PyCodeObject操作；一般一个全局域内的PyCodeObject都包含在`co_consts`种；

### **PyFrameObject**
PyFrameObject的定义如下：
```C++
class PyFrameObject : public PyVarObject {
  PyFrameObject* f_back; // 执行环境链上的前一个frame
  PyCodeObject* f_code; // code object对象
  PyObject* f_builtins; // builtin名字空间
  PyObject* f_globals; // global名字空间
  PyObject* f_locals; // local名字空间
  PyObject** f_valuestack; // 运行时栈的栈底位置
  PyObject** f_stacktop; // 运行时栈的栈顶位置
  ...
  int f_lasti; // 上一条字节码指令在f_code中的偏移的位置
  int f_lineno; // 当前字节码对应的源代码行
  ...
  PyObject* f_localsplus[1]; // 动态内存，维护（局部变量+cell对象集+free对象集+运行时栈）所需的空间
};
```
