# **异常**
***



## **9. 使用析构函数防止资源泄露**
    1) 使用auto_ptr代替raw指针, 可以在处理内部异常的情况下清除资源
        void ProcessAdoptions(istream& data_source) {
          while (data_source) {
            auto_ptr<ALA> pa(ReadALA(data_source));
            pa->ProcessAdoption();
          }
        }
       这样pa->ProcessAdoption()抛出异常的时候, 也可以正常清理资源;
    2) 对于C-like的接口, 需要释放资源可以使用类似如下方式:
        class WindowHandle : private UnCopyable {
          WINDOW_HANDLE w_;
        public:
          explicit WindowHandle(WINDOW_HANDLE w) 
            : w_(w) {
          }

          ~WindowHandle(void) {
            DestroyWindow(w_);
          }

          operator WINDOW_HANDLE(void) const {
            return w_;
          }
        };



## **10. 在构造函数中防止资源泄露**
    1) 用对应的auto_ptr对象替换指针成员变量就可以防止构造函数在存在异常时发
       生资源泄露, 就可以不用手工在析构函数中释放资源



## **11. 禁止异常信息传递到析构函数外**
    1) 如果在一个异常被激活的同时, 析构函数也会抛出异常, 并导致程序控制权转
       移到析构函数外, C++将调用terminate函数; 它将终止你程序的运行, 而且是
       立即终止, 甚至连局部对象都没有被释放;
    2) 禁止异常传递到析构函数外有两个原因: 
       * 能在异常转递的堆栈辗转开解的过程中防止terminate调用
       * 能帮助确保析构函数总能完成我们希望它做的所有事情
