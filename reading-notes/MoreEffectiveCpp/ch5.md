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
