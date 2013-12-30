# **让自己习惯C++** #
***


## **01: 视C++为一个语言联邦** ##
    (View C++ as a federation of languages)
    1) 4个语言层次: C/Object-Oriented C++/Template C++/STL
    2) 当从某个次语言切换到另一个, 导致高效编程守则要求你改变策略;
       对(C-like)类型pass-by-value通常比pass-by-refernce高效;
       对(OO C++)由于用户自定义ctor和dtor存在, pass-by-refernce-to-const更好
    3) C++高效编程守则视状况而变化, 取决你使用C++的哪部分



## **02: 尽量以const, enum, inline替换#define** ##
    (Prefer consts, enums, and inlines to #define)
    1) 对于单纯常量, 最好以const对象或enums替换#define
    2) 对于形似函数的宏(macros), 最好改用inline函数替换#define


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
