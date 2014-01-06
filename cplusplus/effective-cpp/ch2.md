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


## **08: 别让异常逃离析构函数** ##
    (Prevent exceptions from leaving destructors.)
    1) 析构函数绝对不要吐出异常, 如果一个被析构函数调用的函数可能抛出异常, 
       析构函数应该捕捉任何异常, 然后吞下它们或结束程序
    2) 如果客户需要对某个操作函数运行期间抛出的异常做出反应, 那class应该提
       供一个普通函数执行该操作


## **09: 绝不在构造和析构函数中调用virtual函数** ##
    (Never call virtual functions during constructions and destructions.)
    1) 在构造和析构期间不要调用virtual函数, 因为这类调用从不下降到子类


## **10: 令operator =返回一个reference to *this** ##
    (Have assignment operators return a reference to *this.)
    1) 令赋值操作符返回一个reference to *this


## **11: 在operator =中处理"自我赋值"** ##
    (Handle assignment to self in operator=.)
    1) 正确安全的做法
        Widget& Widget::operator =(const Widget& rhs)
        {
          if (this == &rhs)
            return *this;
          Bitmap* orig = pb;
          pb = new Bitmap(*rhs.pb);
          delete orig;
          return *this;
        }
    2) 确保当对象自我赋值时operator=有良好行为, 其中技术包括比较"来源对象"
       和"目标对象"的地址, 精心周到的语句顺序, 以及copy-and-swap
    3) 确定任何函数如果操作一个以上的对象, 而其中多个对象是同一个对象时, 其
       行为仍然正确
