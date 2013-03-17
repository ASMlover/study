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


## **3. 程序转化语意学(Program Transformation Semantics)** ##
> ### **3.1 明确的初始化操作(Explicit Initialization)** ###
        已知的定义如下:
        //! C++
        X x0;
        void 
        foo_bar(void)
        {
          X x1(x0);
          X x2 = x0;
          X x3 = X(x0);
        }
        其必要的程序转换有两个阶段
    1) 重写每一个定义, 其中的初始化操作会被剥除
    2) class的拷贝构造函数会被安插进去
> ### **3.2 参数的初始化(Argument Initialization)** ###
        已知定义如下:
        //! C++
        void foo(X x0);
        X xx;
        foo(xx);
    1) 将会要求局部变量x0以按成员的方式将xx作为初始值, 在编译器实现中则导入
       暂时性的object, 并调用拷贝构造将它初始化, 然后将该暂时性的object交给
       函数;
        //! C++(伪代码)
        X __temp0;
        __temp0.X::X(xx);
        foo(__temp0);
    2) 暂时性的object先以类X的拷贝构造正确的设定了初值, 然后再以位(bitwise)
       的方式拷贝到x0这个函数的局部变量中;
    3) 基于上述原因, foo的声明应该如:
        void foo(X& x0);
> ### **3.3 返回值的初始化(Return Value Initialization)** ###
        已知定义如下:
        //! C++
        X 
        bar(void)
        {
          X xx;
          //! ...
          return xx;
        }
        在cfront中的解决方案是一个双阶段转换
    1) 首先加上一个额外参数, 类型是类对象的一个引用; 该参数用来放置被"拷贝构
       建(copy constructed)"而得的返回值
    2) 在return之前插入一个拷贝构造调用操作, 以便将欲传回的object的内容当做
       上述新增参数的初值
        //! C++ (伪码)
        void 
        bar(X& __result) 
        {
          X xx;
          xx.X::X();
          //! ...
          __result.X::X(xx);
          return;
        }
    3) 这样编译器就必须转换每一个bar调用以便反映其新定义
        X xx = bar();
        将被转换为:
        X xx;   //!< 这里不必执行默认构造函数(Default Constructor)
        bar(xx);
> ### **3.4 在使用者层面做优化(Optimization at the User Level)** ###
        例子:
        //! C++
        X
        bar(const T& y, const T& z)
        {
          X xx;
          //! ...
          return xx;
        }
        被改写成如下:
        X
        bar(const T& y, const T& z)
        {
          return X(y, z);
        }
        这样在编译器中bar将会被解释为:
        //! C++(伪码)
        void 
        bar(X& __result, const T& y, const T& z)
        {
          __result.X::X(y, z);
          return;
        }
    1) __result被直接计算出来, 而不需要经由拷贝构造拷贝而得
    2) 其缺点是怕那些特殊计算用途的构造函数可能会大量扩散
    3) 在实际工程中(个人)不建议采取该方式来优化
> ### **3.5 在编译器层面做优化(Optimization at the Compiler Level)** ###
        前面的例子在编译器中优化如下:
        //! C++(伪码)
        void 
        bar(X& __result)
        {
          //! 默认构造函数被调用
          __result.X::X();
          //! ... 直接处理__result
          return;
        }
    1) 这样的编译器优化称为Named Return Value(NRV)优化, 所有的return指令传回
       相同的具名数值, 这样编译器就将其中的xx以__result替换掉
    2) 例子(虽然下面3个初始化操作在语意上相等)
        X x0(1024);
        X x1 = x(1024);
        X x2 = (X)1024;
        但是第二和第三个定义确做了两个步骤的初始化操作:
        将一个暂时性的object设置初值为1024;
        将暂时性的object以拷贝构造的方式作为explicit object的初值;
        //! C++(伪码)
        x0.X:X(1024);
        x1和x2确调用了两个构造函数, 产生一个暂时性objec, 并对其调用析构:
        //! C++(伪码)
        X __temp0;
        __temp0.X::X(1024);
        x1.X:X(__temp0);
        __temp0.X::~X();
> ### **3.6 Copy Constructor: 要还是不要?** ###
        已知一个3D坐标类如下:
        class Point3D {
          float x_, y_, z_;
        public:
          Point3D(float x, float y, float z);
        };
    1) 类的默认拷贝构造函数被视为不重要的, 它没有任何成员(或基类)对象带有拷
       贝构造, 也没有任何虚基类或虚函数; 所以其对象的按成员拷贝初始化操作会
       导致按位拷贝
    2) 类的3个成员是以数值类存储, 按位拷贝不会导致内存泄露, 也不会产生地址对
       齐(address aliasing)的问题, 既快捷又安全
    3) 也不需要为该类提供一个明确的拷贝构造, 因为编译器自动实施了最好的行为;



## **4. 成员们的初始化队伍(Member Initialization List)** ##
        下列情况必须使用member initialization list才能让程序顺利编译
    1) 当初始化一个reference member时
    2) 当初始化一个const member时
    3) 当调用一个base class的constructor, 而它拥有一组参数时
    4) 当调用一个member class的constructor, 而它拥有一组参数时
        有如下例子:
        //! C++
        class Word {
          String name_;
          int count_;
        public:
          //! 没有错误, 但是效率确低下...
          Word(void)
          {
            name_ = 0;
            count_ = 0;
          }
        };
        在编译器中可能会被解释成:
        //! C++ (伪码)
        Word::Word(/* this pointer goes here */)
        {
          //! 调用String的默认构造函数
          name_.String::String();
          //! 产生暂时性对象
          String __temp = String(0);
          //! 按成员逐次的方式拷贝name_
          name_.String::operator=(__temp);
          //! 析构临时对象
          __temp.String::~String();
          count_ = 0;
        }
        而如果使用初始化列表后效率将有一个提升:
        //! C++
        Word::Word
          : name_(0)
        {
          count_ = 0;
        }
        编译器扩充将如下:
        //! C++ (伪码)
        Word::Word(/* this pointer goes here */)
        {
          //! 调用String(int v)的构造函数
          name_.String::String(0);
          count_ = 0;
        }
    5) 初始化的次序是由class中的member的声明次序决定的, 不是由初始化列表中的
       排列次序所决定的;
       初始化次序和初始化列表中的成员排列次序之间的外观错乱有时会导致意想不
       到的危险, 如下:
        //! C++
        class X {
          int i;
          int j;
        public:
          X(int val)
            : j(val), i(j)
          {
          }
        };
       这样会导致i是一个无法预知的值
    6) 初始化列表中的项目会被安插到确切的用户代码之前, 所有下面的代码是正确
       的:
        //! C++
        X::X(int val)
          : j(val)
        {
          i = j;
        }
    7) 还可以在初始化列表中调用一个成员函数来设定一个成员的初值
        X::X(int val)
          : i(xfoo(val)), j(val)
        {}
        但是: 请使用存在于构造函数内的一个成员而不要使用存在于成员初始化列表
        中的成员来为另一个成员设定初值; 你并不知道xfoo对X object的依赖性有多
        高, 如果将xfoo放在构造函数内, 那对于"到底是哪一个成员在xfoo执行时被
        设定初值"就很明确了
    8) 编译器会对初始化列表一一处理并可能重新排列, 它会安插一些代码到构造函
       数内, 并放置在任何确切的用户代码之前
