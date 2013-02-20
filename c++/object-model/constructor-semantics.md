# **构造函数语意学(The Semantics of Constructors)** #
***


## **1. Default Constructors的构建操作** ##
        Default Constructor在编译器需要的时候才会编译器产生出来;
        对于Class X, 如果没有任何user-declared Constructor, 那么会有一个
    default Constructor被暗中声明出来, 一个被暗中生命出来的default 
    Constructor将是一个没什么用的constructor ...
        下面开始讲解产生一个无用的default constructor的情况
> ### **1.1 带有Default Constructor的Member Class Object** ###
    1) 如果一个class没任何constructor, 但它内含一个member Object, 而这个内含
       的default Object有默认构造函数, 那么这个class的implicit默认构造函数就
       是一个无用的, 编译器就需要为这个class合成一个默认构造函数, 但是该合成
       只会在构造函数需要的被调用的时候才会发生
    2) 为了避免编译器合成出多个default constructor, 编译器把合成的default 
       constructor/copy constructor/destructor/assignment copy operator都以
       inline的方式; 一个inline的函数是静态的不会被其他文件看到; 而如果函数
       太复杂就会合成出一个expicit non-inline static函数
    3) 如果程序中已经明确定义了一个default constructor, 那么编译器会扩张已经
       存在的constructor, 在其中安插一些代码, 使得user code在被执行之前, 先
       调用必要的default Constructors;
       如果有多个class member Objects都要求constructor初始化操作, 编译器将以
       member Objects在class中声明次序来调用各个Constructors 
> ### **1.2 带有Default Constructor的Base Class** ###
    1) 一个没有任何构造函数的类派生自一个带有默认构造函数的基类, 那么这个派
       生类的默认构造函数会被合成出来, 它将调用上一层的基类默认构造函数(根据
       它们声明的次序)
    2) 如果类中提供了多个构造函数, 但其中却没有default constructor, 编译器会
       扩张现有的每一个constructors, 将用以调用所有必要的默认构造函数的程序
       代码添加进去
> ### **1.3 带有一个Virtual Function的Class** ###
    1) class声明(继承)一个virtual function会合成出default constructor
    2) class派生自一个继承串链, 其中有一个或多个virtual base class会合成出
       default Constructor 
    3) 下面的扩张会在编译期间发生:
       一个virtual function table会被编译器产生出来, 存放class的虚函数地址;
       在每一个class object中, 一个额外的pointer member(vptr)会被编译器合成
       出来, 内含相关的class vtbl的地址;
> ### **1.4 带有一个Virtual Base Class的Class** ###
