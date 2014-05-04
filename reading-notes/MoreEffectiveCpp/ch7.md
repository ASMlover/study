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



## **26. 限制某个类所能产生的对象数量**
    1) 阻止建立某个类的对象, 最容易的方法就是把该类的构造函数声明在类的私有
       域(private)
    2) 带有内部链接的函数可能在程序内被复制, 这种复制也包括函数内的静态对象;
       如果建立一个包含局部静态对象的非成员函数, 可能会使程序的静态对象的拷
       贝超过一个, 因此不要建立包含局部静态数据的非成员函数;


## **27. 要求或禁止在堆中产生对象**
    1) 在堆中建立对象
        * 最好的方法是让析构函数成为private, 构造函数为public, 引入一个专用
          的伪析构函数, 来访问真正的析构函数;
        * 另一种方法是将全部的构造函数都声明成private, 其缺点是一个类经常有
          许多构造函数, 必须将它们都声明成private, 否则这些函数将由编译器生
          成, 构造函数/拷贝构造/缺省构造函数; 编译器生成的函数总是public的;
        * 声明类的析构函数或构造函数来阻止建立非堆对象, 这种方法也禁止了继承
          和包容;
        * 解决继承和包容的方法是将析构函数声明在protected, 构造在public即可;
    2) 判断对象是否在堆上
        class HeapTracked {
          typedef const void* RawAddress;
          static std::set<RawAddress> address_;
        public:
          class MissingAddress {};
          virtual ~HeapTracked(void) {
          }

          static void* operator new(size_t size) {
            void* ptr = ::operator new(size);
            address_.insert(ptr);
            return ptr;
          }

          static void operator delete(void* ptr) {
            std::set<RawAddress>::iterator it = address_.find(ptr);
            if (it != address_.end()) {
              address_.erase(it);
              ::operator delete(ptr);
            }
            else {
              throw MissingAddress();
            }
          }

          bool IsOnHeap(void) const {
            RawAddress* raw_address = dynamic_cast<RawAddress>(this);
            std::set<RawAddress>::iterator it = address_.find(raw_address);
            return (it != address_.end());
          }
        };
        std::set<RawAddress> HeapTracked::address_;
      只要继承这个类就可以知道创建的对象是否是在堆上了;
    3) 禁止堆对象
        * 直接将operator new和operator delete声明为private就可以了;
          但是其会阻碍子类在堆中建立对象;
        * 对于包含不能在堆中创建的对象成员的类是可以在堆上创建的;


## **28. 灵巧指针**
    虽然灵巧指针不能完全替代dumb指针, 但是通常打到同样的代码效果, 使用灵巧指
    针更方便也更安全;


## **29. 引用计数**
    1) 使用引用计数的动机:
        * 简化跟踪堆中的对象的过程;
        * 如果很多对象有相同的值, 将这个值存储多次是无聊浪费的, 更好的方法是
          让所有的对象共享这个值;
    2) 实现引用计数(以String为例子)
        * String值和引用计数之间是一一对应的关系, 需要创建一个类来保存引用计
          数以及其跟踪的值;
        * 在String内部实现一个StringValue来保存具体的String值, 并使用引用计
          数来实现;
          class String {
            struct StringValue {
              int   ref_count;
              bool  shareable;
              char* data;
              StringValue(const char* init_val);
              ~StringValue(void);
            };
            StringValue* value_;
            ...
          };
          String::StringValue::StringValue(const char* init_val) 
            : ref_count(1) 
            , shareable(true) {
            data = new char[strlen(init_val) + 1];
            strcpy(data, init_val);
          }
          String::StringValue::~StringValue(void) {
            delete [] data;
          }
          String::String(const String& x) {
            if (x.value_->shareable) {
              value_ = x.value_;
              ++value_->ref_count;
            }
            else {
              value_ = new StringValue(x.value_->data);
            }
          }
          char& String::operator[](int index) {
            if (value_->ref_count > 1) {
              --value_->ref_count;
              value_ = new StringValue(value_->data);
            }
            value_->shareable = false;
            return value_->data[index];
          }



## **30. 代理类**
    1) 可以通过代理类来实现多维数组
        class Array2D {
        public:
          class Array1D {
          public:
            T& operator[](int i);
            const T& operator[](int i) const;
          };
          Array1D& operator[](int i);
          const Array1D& operator[](int i) const;
        };
    2) 使用代理区分通过operator[]进行的是读操作还是写操作;
       proxy类上需要做的３件事
        * 创建它, 指定它扮演哪个字符;
        * 将它作为赋值操作的目标, 在这种情况下可以将赋值真正作用在它扮演的字
          符上; 这样被使用时, proxy扮演的是左值;
        * 用其他方式使用它; 这时proxy扮演的是右值;
        class String {
        public:
          class CharProxy {
          public:
            CharProxy(String& s, int i);
            CharProxy& operator=(const CharProxy& x);   // left value
            CharProxy& operator=(char c);
            operator char(void) const;                  // right value 
          private:
            String& _theString;
            int     _charIndex;
          };
          const CharProxy operator[](int index) const {
            return CharProxy(const_cast<String&>(*this), index);
          }
          CharProxy operator[](int index) {
            return CharProxy(*this, index);
          }

          friend class CharProxy;
        private:
          RCPtr<StringValue> _value;
        };
    3) proxy类可以完成一些其他方法很难甚至不可实现的行为, 如多维数组, 左右值
       区分, 限制隐式类型转换;
       作为函数返回值, proxy对象是临时对象; 增加了软件的复杂度;
