# **Function语意学** #
***


## **1. 成员的各种调用方式** ##
> ### **1.1 非静态成员函数** ###
    1) C++设计准则之一是, 非静态成员函数至少必须和一般的非成员函数具有相同的
       效率
    2) 成员函数被内化为非成员的形式, 具体转换步骤如下:
        改写函数原型以安插一个额外的参数到成员函数中, 用以提供一个存取管道, 
        使类对象得以调用该函数(额外的参数就是this指针);
        将每一个对非静态数据成员的存取操作修改为经由this指针来存取;
        将成员函数重新写成一个外部函数, 对函数名称进行"manging"处理, 使它在
        程序中成为独一无二的词汇;
> ### **1.2 名称的特殊处理** ###
    1) 一般而言, 成员的名称前面会被加上类名称, 形成独一无二的名字
        //! C++
        class Bar {
        public:
          int ival;
        };
        其中ival经过特殊处理可能是ival_3Bar;
        这样当Bar的子类同样有个ival成员的时候, 则其经过特殊处理后仍是唯一的:
        class Foo : pubilc Bar {
        public:
          int ival;
        };
        那在Foo内部则可能被转化为:
        class Foo {
        public:
          int ival_3Bar;
          int ival_3Foo;
        };
        对成员函数的处理也是如此, 当两个函数重载的时候, 加上函数的参数列表, 
        既可以让成员函数独一无二了:
        class Point {
        public:
          void x(float newX);
          float x(void);
        };
        可能被处理为:
        class Point {
        public:
          void x_5PointFf(float newX);
          float x_5PointFv(void);
        };
> ### **1.3 虚拟成员函数** ###
    1) 如果normalize是一个虚成员函数, 那么下面的调用:
        ptr->normalize();
        将会被转换为:
        (*ptr->vptr[1])(ptr);
        * vptr表示由编译器产生的指针, 指向虚表; 它被安插在每一个"声明有(或继
          承自)一个或多个虚函数"的类对象中;
        * 1表示虚表slot的索引值, 关联到normalize函数
        * 第二个ptr是this指针
> ### **1.4 静态成员函数** ###
    1) 如果normalize是一个静态成员函数, 那么下面的调用操作:
        obj.normalize();
        ptr->normalize();
        会被转换为一般的非成员函数的调用, 如下:
        // obj.normalize();
        normalize_7Point3dSFv();
        // ptr->normalize();
        normalize_7Point3dSFv();
    2) 静态成员函数的主要特征就是它没有this指针
        * 不能够直接存取其类中的非静态成员
        * 不能被声明为const, volatile或virtual
        * 不需要经由类对象才被调用——虽然大部分时候它是这样被调用的
    3) 静态成员函数由于缺乏this指针, 因此差不多等同于非成员函数; 其提供的一
       个好处就是成为一个回调函数
