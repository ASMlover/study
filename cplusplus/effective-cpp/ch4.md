# **设计与声明** #
***


## **18: 让接口容易被正确使用, 不易被误用** ##
    (Make interfaces easy to use correctly and hard to use incorrectly.)
    1) 好的接口很容易被正确使用, 不容易被误用; 你应该在你的所有接口中努力
       达成这些性质
    2) "促进正确使用"的办法包括接口的一致性, 以及与内置类型的行为兼容
    3) "阻止误用"的办法包括建立新类型, 限制类型上的操作, 束缚对象值, 以及 
       消除客户的资源管理责任
    4) shared_ptr支持定制型删除器, 这可防范DLL问题, 可被用来自动解除互斥锁
       等等



## **19: 设计class犹如设计type** ##
    (Treat class design as type design.)
    1) 新type的对象应该如何被创建和销毁?
    2) 对象的初始化和对象的赋值该有什么样的差别?
    3) 新type的对象如何被passed by value, 意味着什么?
    4) 什么是新type的"合法值"?
    5) 你的新type需要配合某个继承图系吗?
    6) 你的新type需要什么样的转换?
    7) 什么杨的操作符和函数对此新type而言是合理的?
    8) 什么杨的标准函数应该驳回?
    9) 谁该取用新type的成员? 
    10) 什么是新type的"未声明接口"?
    11) 你的新type有多么一般化?
    12) 你真的需要一个新type吗? 
    * Classes的设计就是type的设计.



## **20: 宁以pass-by-reference-to-const替换pass-by-value** ##
    (Prefer pass-by-reference-to-const to pass-by-value)
    1) 尽量以pass-by-reference-to-const替换pass-by-value; 前者通常比较高效,
       并可避免切割问题
    2) 以上规则并不适用与内置类型, 以及STL的迭代器和函数对象, pass-by-value
       更合适
