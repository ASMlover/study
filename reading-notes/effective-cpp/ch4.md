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
> ### **个人理解**
    接口的设计应该尽量的保持一致性, 而且应该与内置类型兼容;
    接口的一致性可以降低使用者的学习成本;
        shared_ptr<Investment> CreateInvestment(void) {
          shared_ptr<Investment> r(static_cast<Investment*>(0), 
                                   ReleaseInvestment);
          r = ...;
          return r;
        }
        这样的设计可以避免DLL中返回的对象造成的资源泄露;



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
> ### **个人理解**
    我们在设计一个类的时候, 应该仔细考虑该类的必要性以及其行为应该参考内置类
    行的设计;



## **20: 宁以pass-by-reference-to-const替换pass-by-value** ##
    (Prefer pass-by-reference-to-const to pass-by-value)
    1) 尽量以pass-by-reference-to-const替换pass-by-value; 前者通常比较高效,
       并可避免切割问题
    2) 以上规则并不适用与内置类型, 以及STL的迭代器和函数对象, pass-by-value
       更合适
> ### **个人理解**
    在C++编译器底层, 引用是以指针来实现的, 因此pass by reference意味着传递的
    是指针, 因此对于C++的内置类型来说pass by value的效率更高;
    对于自定义了类, 尽量以const引用来传递, 而对内置类型, STL的迭代器和函数对
    象应该用值类型来传递;



## **21: 必须返回对象时, 别妄想返回其reference** ##
    (Do not try to return a reference when you must return an object.)
    1) 任何函数返回一个reference指向某个local对象, 都会一败涂地
    2) 一个"必须返回新对象"的函数的正确写法是: 就让那个函数返回一个新对象
    3) 当你必须在"返回一个reference和返回一个object"之间抉择时, 你的工作就是
       挑出行为正确的那个
    4) 绝不要返回pointer或reference指向一个local stack对象, 或返回reference
       指向一个heap-allocated对象, 或返回pointer或reference指向一个local 
       static对象而有可能同时需要多个这样的对象



## **22: 将成员变量声明为private** ##
    (Declare data members private.)
    1) 切记将成员变量声明为private, 这可赋予客户访问数据的一致性, 可细微划分
       访问控制, 允诺约束条件获得保证, 并提供class作者以充分的实现弹性
    2) protected并不比public更具封装性



## **23: 宁以non-member, no-friend替换member函数** ##
    (Prefer non-member non-friend functions to member functions.)
    1) 宁可拿non-member non-friend函数替换member函数, 这样做可以增加封装性,
       包裹弹性和技能扩充性



## **24: 若所有参数皆需要类型转换, 请为此采用non-member函数** ##
    (Declare non-member functions when type conversions should apply to all 
     parameters.)
    1) 如果你需要为某个函数的所有参数(包括被this指针所指的那个隐喻参数)进行
       类型转换, 那么这个函数必须是个non-member 



## **25: 考虑写出一个不抛出异常的swap函数** ##
    (Consider support for a non-throwing swap.)
    1) 可以全特化std内的templates, 但是不可以添加新的templates(classes或者
       functions或其他任何东西)到std里头, std的内容完全由C++标准委员会决定
    2) 当std::swap对你的类型效率不高时, 提供一个swap成员函数, 并确定这个函数
       不抛出异常
    3) 如果你提供一个member swap, 也该提供一个non-member swap来调用前者; 对
       于classes(而非templates), 也请特化std::swap
    4) 调用swap时应针对std::swap使用using声明, 然后调用swap并且不带任何"命名
       空间资格修饰"
    5) 为"用户定义类型"进行std templates全特化是好的, 但千万不要尝试在std内
       加入某些对std而言全新的东西
