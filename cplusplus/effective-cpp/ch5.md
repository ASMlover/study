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



## **29: 为"异常安全"而努力是值得的** ##
    (Strive for exception-safe code.)
    1) 异常安全函数即使发生异常也不会泄漏资源或允许任何数据结构败坏; 这样的
       函数却分为3中可能的保证: 基本型, 强烈型, 不抛出异常型
    2) "强烈保证"往往以copy-and-swap实现, 但是"强烈保证"并非对所有函数都可实
       现或具备现实意义
    3) 函数提供的"异常安全保证"通常最高只等于其所调用之各个函数的"异常安全
       保证"中最弱的



## **30: 透彻了解inlining的里里外外** ##
    (Understand the ins and outs of inlining.)
    1) inline只是对编译器的一个申请, 不是强制命令
    2) 将大多数inlining限制在小型, 被频繁调用的函数上; 这可使日后调试和二进
       制升级更容易, 也可使潜在的代码膨胀问题最小化, 使程序的速度提升机会最
       大化
    3) 不要只因为function templates出现在头文件, 就将它们声明为inline



## **31: 将文件间的编译依存关系将至最低** ##
    (Minimize compilation dependencies between files.)
    1) 如果使用object references或object pointers可以完成任务, 就不要使用
       objects
    2) 如果能够, 尽量以class声明式替换class定义式
       当你声明一个函数而它用到某个class时, 你并不需要该class的定义; 纵使函
       数以by value方式传递该类型的参数亦然;
    3) 为声明式和定义式提供不同的头文件
    4) 支持"编译依存性最小化"的一般构想是: 相依于声明式, 不要相依于定义式; 
       基于此构想的两个手段是Handle classes和Interface classes
    5) 程序库头文件应该以"完全且仅有声明式"的形式存在, 这种做法不论是否涉及
       templates都适用
