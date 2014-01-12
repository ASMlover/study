# **实现** #
***



## **26: 尽可能延后变量定义式的出现时间** ##
    (Postpone variable definitions as long as possible.)
    1) 尽可能咽喉变量定义式的出现, 这样做可增加程序的清晰度并改善程序效率



## **27: 尽量少做转型动作** ##
    (Minimize casting.)
    1) const_cast通常被用来将对象的常量性转除, 它也是唯一有此能力的C++-style
       转型操作符
    2) dynamic_cast主要用来执行"安全向下转型", 也就是用来决定某对象是否归属
       继承体系中的某个类型; 它是唯一无法由旧式语法执行的动作, 也是唯一可能
       耗费重大运行成本的转型动作
    3) reinterpret_cast意图执行低级转型, 实际动作可能取决于编译器, 这样表示
       它不可移植
    4) static_cast用来强迫隐式转换, 可以用来指向上述多种转换的反向转换, 但
       不能将const转换为non-const(const_cast可以办到)
    5) 如果可以, 尽量避免转型, 特别是在注重效率的代码中避免dynamic_cast; 如
       果有个设计需要转型动作, 试着发展无需转型的替代设计
    6) 如果转型是必须, 试着将它隐藏与某个函数背后; 客户随后可以调用该函数, 
       而不需将转型放进他们自己的代码内
    7) 宁可使用C++-style转型, 不要使用旧式转型


## **28: 避免返回handles指向对象内部成分** ##
    (Avoid returning "handles" to object internals.)
    1) 避免返回handles(包括references, 指针, 迭代器)指向对象内部
