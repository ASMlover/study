# **继承与面向对象设计** #
***



## **32: 确定你的public继承塑膜出is-a关系** ##
    (Make sure public inheritance models "is-a".)
    1) "public继承"意味is-a; 适用于基类身上的每一件事情一定也适用于子类身上,
       因为每一个子类对象也都是一个基类对象



## **33: 避免遮掩继承而来的名称** ##
    (Avoid hiding inherited names.)
    1) 子类内的名称会遮掩基类内的名称, 在public继承下从来没人希望如此
    2) 为了让被遮掩的名称在见天日, 可以使用using声明式或转交函数


## **34: 区分接口继承和实现继承** ##
    (Differentiate between inheritance of interface and inheritance of 
     implementation.)
    1) 以public形式的继承
        * 成员函数的接口总是会被继承
        * 声明一个纯虚函数的目的是为了让子类只继承函数接口
        * 声明简朴的非纯虚函数的目的是让子类继承该函数的接口和缺省实现
    2) 声明非虚函数的目的是为了令子类继承函数的接口以及一份强制性实现
    3) 接口继承和实现继承不同, 在public继承之下, 子类总是继承基类的接口
    4) 纯虚函数只是具体指定接口继承
    5) 简朴(非纯)虚函数具体指定接口继承以及缺省实现继承
    6) 非虚函数具体指定接口继承以及强制性实现继承



## **35: 考虑虚函数以外的其他选择** ##
    (Consider alternatives to virtual functions.)
    1) 虚函数的替代方案包括NVI手法以及Strategy设计模式的多种形式, NVI手法自
       身是一个特殊形式的Template Method设计模式
    2) 将机能从成员函数移到class外部函数, 带来的一个缺点是, 非成员函数无法访
       问class的non-public成员
    3) tr1::function对象的行为就像一般函数指针, 这样的对象可接纳"与给定之目
       标签名式兼容"的所有可调用物
