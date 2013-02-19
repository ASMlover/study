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
