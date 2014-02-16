# **构造/析构/赋值运算** #
***


## **05: 了解C++默默编写并调用哪些函数** ##
    (Know what functions C++ silently writes and calls)
    1) 编译器可以暗自为class创建default构造函数, copy构造函数, 
       copy assignment操作符, 以及析构函数
    2) 但它们是在被需要的时候被编译器创建出来
> ### **个人理解**
    默认构造, 析构, 拷贝构造, 赋值会在被调用(需要)时被编译器创建;
    编译器默认生成的析构是非虚的, 除非其基类有虚析构函数;
    C++不允许让引用改指向不同的对象, 因此如果类中用引用成员或const成员, 
    编译器将拒绝创建拷贝赋值;


## **06: 若不想使用编译器自动生成的函数, 就该明确拒绝** ##
    (Explicit disallow the use of compiler-generated functions 
     you do not want)
    1) 为驳回编译器自动提供的机能, 可将相应的成员函数声明为private并且不予
       实现; 使用像Uncopyable这样的base class也是一种做法
    2) 使用Uncopyable这样的基类可以阻止成员函数或friend函数尝试拷贝对象
> ### **个人理解**
    将未实现的拷贝构造和赋值函数的声明放在基类的private, 可以将错误提示移动
    到编译期;


## **07: 为多态基类声明virtual析构函数** ##
    (Declare destructors virtual in polymorphic base class)
    1) virtual函数的目的是允许子类的实现得以自行定制化
    2) polymorphic(带多态性质的)基类应该声明一个virtual析构函数, 如果class
       带有任何virtual函数, 它就应该拥有一个virtual析构函数
    3) 类的设计目的如果不是作为基类使用, 或不是为了具备多态性, 就不应该声明
       virtual析构函数 
> ### **个人理解**
    如果基类没有定义虚析构, 那么销毁基类指针所指的对象的时候, 那么子类的成分
    很可能没有被销毁, 那子类的析构函数也可能并没有执行, 而基类成分会被销毁,
    会造成局部销毁, 会造成资源泄露的问题;
    虚函数的目的允许子类的实现得以定制化;
    可以为类设置一个纯虚析构函数, 但是必须为其添加一个实现(./demo/rule7/);


## **08: 别让异常逃离析构函数** ##
    (Prevent exceptions from leaving destructors.)
    1) 析构函数绝对不要吐出异常, 如果一个被析构函数调用的函数可能抛出异常, 
       析构函数应该捕捉任何异常, 然后吞下它们或结束程序
    2) 如果客户需要对某个操作函数运行期间抛出的异常做出反应, 那class应该提
       供一个普通函数执行该操作
> ### **个人理解**
    析构函数就不应该抛出异常, 要么析构捕获异常就结束程序或者直接吞下该异常,
    继续执行;
    如果某个操作可能抛出异常, 就应该提供一个函数接口以便将处理异常的权利交给
    用户;


## **09: 绝不在构造和析构函数中调用virtual函数** ##
    (Never call virtual functions during constructions and destructions.)
    1) 在构造和析构期间不要调用virtual函数, 因为这类调用从不下降到子类
> ### **个人理解**
    在构造和析构中调用虚函数, 这个时候有可能子类还没有构建或者子类资源已经释
    放了, 而造成调用期错误~~~


## **10: 令operator =返回一个reference to *this** ##
    (Have assignment operators return a reference to *this.)
    1) 令赋值操作符返回一个reference to *this
> ### **个人理解**
    因为赋值需要实现连锁赋值, 赋值操作必须返回一个引用指向操作符的左侧实参;
    同时所有赋值相关的操作也应该遵守该准则;


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
> ### **个人理解**
    如果在赋值操作中没有处理自我赋值的问题, 有可能造成该对象被删除的问题;
    想要实现没有异常的赋值函数, 可以使用copy-and-swap技术(这样的技术可能会
    造成一定的性能消耗);



## **12: 复制对象时勿忘其每一个成分** ##
    (Copy all parts of an object.)
    1) 任何时候只要你承担起为子类编写copying函数的责任, 必须很小心的复制其
       基类成分, 那些成分往往是私有的, 你应该让子类的copying函数调用相应的
       基类成分
        PriorityCustomer::PriorityCustomer(const PriorityCustomer& rhs)
          : Customer(rhs)
          , priority_(rhs.priority_)
        {
        }

        PriorityCustomer& 
        PriorityCustomer::operator=(const PriorityCustomer& rhs)
        {
          Customer::operator=(rhs);
          priority_ = rhs.priority_;
          return *this;
        }
    2) 当你编写一个copying函数, 请确保
        * 复制所有local成员变量
        * 调用所有基类的适当copying函数 
    3) copying函数应该确保复制对象内所有成员变量和所有基类成分
    4) 不要尝试以某个copying函数使用另一个copying函数, 应该将共同机能放到第
       3个函数中, 由2个copying函数共同调用
