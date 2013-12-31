# **构造/析构/赋值运算** #
***


## **05: 了解C++默默编写并调用哪些函数** ##
    (Know what functions C++ silently writes and calls)
    1) 编译器可以暗自为class创建default构造函数, copy构造函数, 
       copy assignment操作符, 以及析构函数
    2) 但它们是在被需要的时候被编译器创建出来


## **06: 若不想使用编译器自动生成的函数, 就该明确拒绝** ##
    (Explicit disallow the use of compiler-generated functions 
     you do not want)
    1) 为驳回编译器自动提供的机能, 可将相应的成员函数声明为private并且不予
       实现; 使用像Uncopyable这样的base class也是一种做法
    2) 使用Uncopyable这样的基类可以阻止成员函数或friend函数尝试拷贝对象


## **07: 为多态基类声明virtual析构函数** ##
    (Declare destructors virtual in polymorphic base class)
    1) virtual函数的目的是允许子类的实现得以自行定制化
    2) polymorphic(带多态性质的)基类应该声明一个virtual析构函数, 如果class
       带有任何virtual函数, 它就应该拥有一个virtual析构函数
    3) 类的设计目的如果不是作为基类使用, 或不是为了具备多态性, 就不应该声明
       virtual析构函数
