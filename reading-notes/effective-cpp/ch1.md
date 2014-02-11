# **让自己习惯C++** #
***
***


## **01: 视C++为一个语言联邦** ##
    (View C++ as a federation of languages)
    1) 4个语言层次: C/面向对象/模板元编程/STL
    2) 当从某个次语言切换到另一个, 导致高效编程守则要求你改变策略;
       对(C-like)类型pass-by-value通常比pass-by-refernce高效;
       对(OO C++)由于用户自定义ctor和dtor存在, pass-by-refernce-to-const更好
    3) C++高效编程守则视状况而变化, 取决你使用C++的哪部分
> ### **个人理解** ###
    在各个语言层次遵守各自的编程守则;
    C部分传值比引用高效;
    面向对象部分由于构造析构导致引用传递更好, 尤其是模板元编程时甚至不知所
    处理对象的类型;
    STL部分迭代器和函数对象是在C指针之上构建起来的, C的传值效果更好;



## **02: 尽量以const, enum, inline替换#define** ##
    (Prefer consts, enums, and inlines to #define)
    1) 对于单纯常量, 最好以const对象或enums替换#define
    2) 对于形似函数的宏(macros), 最好改用inline函数替换#define
> ### **个人理解** ###
    宏只是简单的替换(在预处理器阶段), 并非真正的编译, 因此出错时在符号表中并
    不知道所定义的符号;
    以常量替换宏时: 定义常量指针(指针和值都应该是常量, 可使用std::string), 
    定义类专属常量必须声明为static(如果要取该变量的地址就必须提供定义式)
    (这部分例子请见./demo/rule2);
    如果不希望对常量使用指针或引用, 使用enum;
    对于宏函数, 使用inline模板, 既不需要在函数中为参数加上括号, 也不需要操心
    参数被核算多次;


## **03: 尽可能使用const** ##
    (Use const whenever possible)
    1) STL中迭代器的作用就像T*指针, 声明迭代器为const就像声明指针为const一样
       表示这个迭代器不能指向不同的东西, 但是它指向的东西的值是可改变的, 如
       果希望迭代器所指向的东西不可被改变, 就需要const_iterator
    2) const成员函数, 只有在不更改对象的任何成员变量时才可以说是const, 但是
       也有例外可以通过编译的列子:
          class TextBlock {
            char* text_;
          public:
            char& operator [](size_t pos) const { 
              return text_[pos];
            }
          };
          const TextBlock cctb("Hello");
          char* pc = &cctb[0];
          *pc = 'J';
    3) 对于bitwise-constness非我所欲的问题, 可以使用mutable来解决
          class TextBlock {
            char*  text_;
            mutable size_t length_;     //size_t length_;
            mutable bool length_valid_; //bool   length_valid_;
          public:
            size_t length(void) const {
              if (!length_valid_) {
                length_ = strlen(text_);
                length_valid_ = true;
              }
              return length_;
            }
          };
          对于length_和length_valid_被改变, 可以使用mutable来修饰
    4) 将某些东西声明为const可帮助编译器检查出错误用法, const可被用在任何作
       用域内的对象, 函数参数, 函数返回类型, 成员函数本身
    5) 编译器强制实施bitwise constness, 但是你编写的程序应该使用概念上的常量
       性
    6) 当const和non-const成员函数等价实现的时候, 令non-const版本调用const版 
       本可避免代码重复
> ### **个人理解** ###
    将函数返回值设置为const是为了阻止用户对返回值做一些赋值的暴行;
    const成员函数不可更改对象内任何非静态成员变量;
    mutable释放掉非静态成员变量的bitwise constness约束;


## **04: 确定对象被使用前已先被初始化** ##
    (Make sure that objects are initialized befored they're used)
    1) 为避免在对象初始化之前过早使用它们; 第一, 手工初始化内置类型的
       non-member对象; 第二, 使用成员初始化列表对付对象的所有成分; 最后, 在
       初始化次序不确定的情况下, 加强你的设计
    2) 为内置类型的对象进行手工初始化, C++不保证初始化它们
    3) 构造函数最好使用成员初始化列表, 而不要在构造函数内部使用赋值操作, 初
       始化列表中的成员变量, 其排列次序应该和它们在class中的声明次序相同
    4) 为避免"跨编译单元初始化次序"问题, 使用local static对象替换non-local 
       static对象 
> ### **个人理解** ###
    总是使用成员初始化列表, 这样做有时绝对必要, 又比赋值更高效;
    对于不清楚跨编译单元的变量的初始化次序, 可以使用单件模式来实现;
