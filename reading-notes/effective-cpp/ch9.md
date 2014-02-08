# **杂项讨论** #
***




## **53: 不要轻忽编译器的警告** ##
    (Pay attention to compiler warnings.)
    1) 严肃对待编译器发出的警告信息, 努力在你的编译器的最高警告级别下争取
       "无任何警告"的荣誉
    2) 不要过度依赖编译器的报警能力, 因为不同的编译器对待事情的态度并不相同;
       一旦移植到另一个编译器上, 你原本依赖的警告信息有可能消失



## **54: 让自己熟悉包括tr1在内的标准程序库** ##
    (Familiarize yourself with the standard library, including tr1.)
    1) C++98的标准程序库的主要成分
       * STL, 容器(如vector, string, map, ...), 迭代器(iterators), 算法(
         algorithms如find, sort, transform), 函数对象(如less, greater), 各种
         容器适配器(stack, priority_queue)和函数对象适配器(mem_fun, not1)
       * IO streams, 用户自定缓冲功能,国际化I/O, 以及预定义好的对象cin,cout,
         cerr和clog
       * 国际化支持, wchar_t, wstring ...
       * 数值处理, complex, valarray
       * 异常阶层体系, 基类exception和期子类logic_error和runtime_error, 以及
         更深继承的各个类
       * C89标准程序库
    2) tr1
       * 智能指针 tr1::shared_ptr和tr1::weak_ptr
       * tr1::function 
       * tr1::bind
       * hash tables
          tr1::unordered_set, tr1::unordered_multiset, tr1::unordered_map
          tr1::unordered_multimap
       * 正则表达式
       * tuples
       * tr1::array
       * tr1:mem_fn
       * tr1::reference_wrapper
       * 随机数生成工具
       * 数学特殊函数
       * C99兼容扩充
       * type traits
       * tr1::result_of
    3) C++标准程序库的主要既能由STL, iostreams, locales组成; 并包含C99标准
       程序库
    4) tr1添加了智能指针(tr1::shared_ptr), 一般化函数指针(tr1::function),
       hash-based容器, 正则表达式以及另外10个组件的支持
    5) tr1自身只是一份规范, 为获得tr1提供的好处, 你需要一份实物 -> boost



## **55: 让自己熟悉boost** ##
    (Familiarize yourself with boost.)
    1) boost是一个社群, 也是一个网站; 致力于免费, 源码开放, 同僚复审的C++程
       序库开发, boost在C++标准化过程中扮演深具影响力的角色
    2) boost提供许多tr1组件的实现品, 以及其他许多程序库
