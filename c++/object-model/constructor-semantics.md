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
        class X { public: int i; };
        class A : public virtual X { public: int j; };
        class B : public virtual X { public: double d; };
        class C : public A, public B { public: int k; };
        //! 无法在编译期间决定pa->X::i的位置
        void foo(const A* pa) { pa->i = 1024; }
    1) 编译时, foo可能被改写为:
       void foo(const A* pa) { pa->__vbcX->i = 1024; }
    2) __vbcX表示编译器所产生的指针, 指向virtual base class X
    3) 如果class没有声明任何Constructors, 那么编译器必须为它合成一个默认的
       Constructor 
> ### **1.5 总结** ###
    1) 被合成出来的Constructor只能满足编译器的需要, 是借"调用member object 
       或base class的默认构造函数"或是"为每一个Object初始化其虚函数机制或虚
       基类机制"而完成的
    2) 这4种情况之外且没有声明任何构造函数的类, 则他们拥有隐式的非必要的默认
       构造函数, 实际是不会被合成出来的
    3) 合成的默认构造函数中, 只有base class subobjects和member class objects
       会被初始化, 其他的非静态数据成员(函数, 函数指针, 整数数组等)都不会被
       初始化 



## **2. Copy Constructor的构建操作** ##
        有3种情况, 会以一个object的内容作为另一个class object的初始值, 即是
    要对一个object做明确的初始化操作:
        class X { ... };
        X x;

        其中一种情况是对一个object做明确的初始化操作:
        //! C++
        X xx = x;

        另两种情况是当object被当做参数交给某个参数时:
        extern void foo(X x);
        void 
        bar(void)
        {
          X xx;
          foo(xx);
        }

        还有一种是当函数传回一个class object时:
        x 
        foo_bar(void)
        {
          X xx;
          //! ... 
          return xx;
        }
> ### **2.1 缺省按成员初始化(Default Memberwise Initialization)** ###
    1) 如果class没有提供一个显示的拷贝构造, 当class object以"相同的class的另
       一个object"作为初值, 其内部是以缺省按成员初始化的手法来完成的, 即是把
       每一个内建的或派生的data member的值, 从某个对象拷贝一份到另一个对象身
       上
    2) 默认构造函数和拷贝构造函数会在必要的时候才由编译器产生出来
    3) 一个类对象可以从两种方式复制得到, 一种是被初始化话, 另一种是被指定, 
       这两个操作分别是以拷贝构造函数和拷贝赋值操作完成的
> ### **2.2 位逐次拷贝(Bitwise Copy Semantics)** ###
        //! C++
        Word a("book");
        Word b = a;
    1) 如果Word定义了一个拷贝构造, b的初始化就会去调用它; 如果Word没有定义一
       个明确的拷贝构造, 那编译器就会视Word是否展现位逐次拷贝来决定是否合成
       一个默认拷贝构造函数了
    2) 如果Word的实现如下, 展现除了默认拷贝的语义就不需要合成出一个缺省拷贝
       构造了(b的初始化操作也就不需要以一个函数调用来完成了):
        //! C++
        class Word {
          int count;
          char* str;
        public:
          Word(const char* s);
          ~Word(void) { delete [] str; }
        };
    3) 如果Word的声明如下, 而String声明了一个确切的拷贝构造, 那么编译器就会
       合成出一个Word的拷贝构造一边调用类成员String的拷贝构造:
        // C++
        class Word {
          int count;
          String str;
        public:
          Word(const String& s);
          ~Word(void);
        };
        class String {
        public:
          String(const char* s);
          String(const String& s);
          ~String(void);
          //! ...
        };
       编译器合成出来的拷贝构造大致如下(伪代码):
        inline 
        Word:Word(const Word& w)
        {
          str.String::String(w.str);
          count = w.count;
        }
    4) 在被合成出来的拷贝构造中, (整数, 指针, 数组等的nonclass members)也都
       会被复制
> ### **2.3 不要位逐次拷贝** ###
    一个class不展现位逐次拷贝的情况如下:
    1) class内含一个对象成员, 切该对象成员的class声明有一个拷贝构造函数
    2) 当class继承自一个有拷贝构造函数的基类
    3) class声明了一个或多个虚函数
    4) class继承的基类中, 有一个或多个虚基类
        在前两种情况中, 编译器必须将成员或基类的拷贝构造的调用安插到合成的拷
    贝构造函数当中
> ### **2.4 重新设定Virtual Table的指针** ###
    1) 编译期间只要有一个类声明了虚函数就会做如下扩张:
        增加一个虚函数表(vtbl), 内含每一个有作用的虚函数地址;
        将一个指向虚函数表的指针(vptr), 安插到每一个类对象中;
    2) 当编译器导入一个vptr到class之中时, 该class就不再展现位逐次语义了
> ### **2.5 处理Virtual Base Class Subobject** ###
    1) 一个类对象如果以另一个对象作为初值, 而后者有一个虚基类的Subobject, 就
       会使"位逐次拷贝"失效
    2) 编译器必须让"继承的类对象"中的"虚基类subobject的位置"在执行期间准备妥
       当; 维护"位置完整性"是编译器的责任, "位逐次拷贝"可能会破坏这个位置, 
       所以编译器必须在它自己合成才出来的拷贝构造函数中做出仲裁
    3) 对于所讲述的4种编译器会合成拷贝构造函数的情况, 在那些情况下class不再
       保持"位逐次拷贝", 而且默认拷贝构造函数未被声明的话会被认为是不重要的;
       在这4种情况下, 如果缺失一个已经声明的拷贝构造函数, 编译器为了正确处理
       "以一个class object作为另一个class object的初值", 必须合成出一个拷贝
       构造函数
