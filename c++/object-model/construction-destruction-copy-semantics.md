# **构造, 解析, 拷贝语意学** #
*** 

## **1. 引子** ##
        //! C++
        class Abstract_base {
        public:
          virtual ~Abstract_base(void) = 0;
          virtual void interface(void) const = 0;
          virtual const char* mumble(void) const { return mumble_; }
        protected:
          char* mumble_;
        };
> ### **1.1 纯虚函数的存在** ###
    1) 纯虚析构函数, 类的设计者一定得定义它; 因为每一个子类的析构函数会被编
       译器加以扩展, 以静态调用的方式调用其每一个虚基类以及上一层基类的析构
       函数; 因此, 只要缺乏任何一个基类析构函数的定义, 都会导致连接失败... 
    2) 不要将虚析构函数声明为纯虚的 
> ### **1.2 虚拟规格的存在** ###
        将所有的成员函数都声明为虚函数, 然后再靠编译器的优化操作把非必要的虚
    函数去除, 是一个不好的设计观念;
> ### **1.3 虚拟规格中的const的存在** ###
        为一个抽象基类的虚函数设置const的时候, 确实很难决定; 不将函数声明为
    const, 该函数不能获得以一个const引用或const指针; 然而声明一个函数为const
    却发现其构造实例必须修改某一个成员数据;
        所以最好不在抽象基类中的虚函数使用const就是了
> ### **1.4 重新设计的类声明** ###
        //! C++
        class Abstract_base {
        public:
          virtual ~Abstract_base(void);
          virtual void interface(void) = 0;
          const char* mumble(void) const 
          {
            return mumble_;
          }
        protected:
          Abstract_base(char* pc = NULL);
          char* mumble_;
        };
