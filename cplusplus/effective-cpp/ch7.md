# **模板与范型编程** #
***




## **41: 了解隐式接口和编译期多态** ##
    (Understand implicit interfaces and compile-time polymorphism.)
    1) 类和模板都支持接口和多态
    2) 对类而言接口是显式的, 以函数签名为中心; 多态则时通过虚函数发生于运行
       期
    3) 对模板参数而言, 接口是隐式的, 奠基于有效表达式; 多态则是通过模板具现
       化和函数重载解析发生于编译期



## **42: 了解typename的双重意义** ##
    (Understand the two meanings of typename.)
    1) 声明template参数时, 前缀关键字class和typename可互换
    2) 请使用关键字typename标识嵌套从属类型名称; 但不得在基类列表或成员初始
       化列表内以它作为基类的修饰符



## **43: 学习处理模板化基类内的名称** ##
    (Know how to access names in tempatized base classes.)
    1) 在derived class templates内通过"this->"指涉基类模板内的成员名称, 或
       籍由一个明白写出的"子类资格修饰符"完成
