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
