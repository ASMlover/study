# **技巧(Techniques, 又称Idioms或Pattern)**
***



## **25. 将构造函数和非成员函数虚拟化**
    1) 可以让类实现一个虚拟的拷贝函数, 来返回对象的指针;
        class TextBlock : public NLComponent {
        public:
          virtual TextBlock* Clone(void) const {
            return TextBlock(*this);
          }
        };
        让拷贝的含义与真正的拷贝构造函数相同;
    2) 具有虚拟行为的非成员函数很简单, 用一个虚拟函数来完成工作, 然后用一个
       非虚拟函数来调用这个虚拟函数就可以了;
