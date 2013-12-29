# **MemCheck** #
***


## **What's MemCheck?** ##
 * 在VC的Debug模式下, 检查malloc或则new使用的内存泄漏问题


## **注意** ##
 * demo中的new宏重载只能用于最普通的new, 如果是placement new就会编译失败, 所以请确保new的替代宏放在所有标准include头文件的最后


## **判断某个独立函数有没有内存泄漏** ##
    class DbgMemLeak {
      _CrtMemState check_point_;

      DbgMemLeak(const DbgMemLeak&);
      DbgMemLeak& operator(const DbgMemLeak&);
    public:
      explicit DbgMemLeak(void)
      {
        _CrtMemCheckPoint(&check_point_);
      }

      ~DbgMemLeak(void)
      {
        _CrtMemState check_point;
        _CrtMemCheckPoint(&check_point);

        _CrtMemState diff;
        _CrtMemDifference(&diff, &check_point_, &check_point);
        _CrtMemDumpStatistics(&diff);
        _SrtMemDumpAllObjectsSince(&diff);
      }
    };
