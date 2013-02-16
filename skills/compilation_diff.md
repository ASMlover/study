# **不同编译器下C++的差异** #
*** 

## **1. StaticAssert** ##
        主要记录了不同编译器下, 模板函数中调用StaticAssert<false>的差异, 以
    下是一个典型的StaticAssert实现:
        template <bool> struct StaticAssert;  
        template <> struct StaticAssert<bool> {};
> ### **1.1 MSVC & GCC** ###
    1) 在一个为调用的模板函数中调用 StaticAssert<false>();
    2) 这么做的目的大多数情况下是要屏蔽某个函数, 禁止客户调用该函数
    3) 在MSVC和GCC中是可以编译通过的
> ### **1.2 clang** ###
    1) 环境如上
    2) 但是编译会出错, 会显示调用了未实现的类
    3) (估计)clang对C++的检查更为严格, 其内部帮我们分析了模板函数中的错误写法
    4) 但是, 这种时候我们是需要禁止使用那个函数的, 所以使用clang并未达到我们的预期目标
    5) 可能clang内部有个编译选型可以实现该情况(目前我并未得知)




## **2. for** ##
> ### **2.1 MSVC** ###
    1) 在MSVC6.0以上的版本, 理论上for中定义的变量只在for作用范围内有效
    2) 但是项目中出现for中定义的变量在作用域范围后依然在使用
    3) 可以使用编译选项/Zc:forScope强制符合for循环的局部范围
> ### **2.2 gcc & clang** ###
    1) gcc和clang都遵守C++的标准规范
    2) for循环的变量保持在for循环的局部范围
> ### **2.3 结论** ###
    为了程序跨平台, 建议遵守C++语言规范, 将for循环中的变量局限在for作用范围内 




## **3. `__argc & __argv`** ##
        命令行参数, 用于获取程序运行的时候输入的参数信息
> ### **3.1 MSVC** ###
    1) MSVC的stdlib.h提供了几个全局变量, 存储了程序运行时的参数信息
    2) 但是在标准C中这几个变量并非标准
    3) __argc存储了main参数的argc, __argv存储了main参数的argv
> ### **3.2 gcc & clang** ###
    1) 在gcc和clang所提供的stdlib.h中并未提供这些变量
    2) 但是我们可以在程序中使用全局变量来替换掉
> ### **3.3 结论** ###
    1) 建议在程序开发过程中不要使用这些非C/C++标准的东西
    2) 建议在开发跨平台程序的时候, 不要为了偷懒使用一些平台所单独提供的特性、变量等
    3) 这里建议在main函数中, 将命令行参数信息记录到单独的其他全局变量中, 以便使用, 或者分平台单独使用




## **4. 初始化列表的成员初始化顺序** ##
        // C++
        class ExampleClass {
          int x_;
          int y_;
        public:
          ExampleClass(int x,  int y) : y_(y), x_(y_) {}
        };
        C++初始化类成员的时候, 是按照声明的顺序进行初始化的, 而不是按出现在
    初始化列表的顺序。
        你可能以为上面两个成员变量的值是相同的, 但是编译器会先初始化`x_`, 再
    初初始化`y_`的值, 因为他们是按这个顺序声明的。所以`x_`的结果可能是一个不
    可预知的值。  
> ### **结论** ###
    1) 按照你希望初始化的顺序来声明成员
    2) 如果你使用初始化列表, 总是按照他们的声明顺序来罗列这些成员
    3) 在gcc和clang中这是一个警告, 为了避免出错建议使用1), 2)来使用初始化列表



## **5. 函数参数类型匹配** ##
        // C++ 
        static void 
        func_parameter_passing(unsigned long* arg) 
        {
        } 
        static void 
        test_parameter_matching(void) 
        {
          int val = 55;
          func_parameter_passing(&val);
        }
        上面的例子展示了一个传递参数的时候与函数的形参类型不匹配的情况, 在
    MSVC中, 上面的例子是可以通过的, 然而在gcc和clang中并非如此
> ### **5.1 gcc** ###
    1) 如果是在C语言中, 用gcc来编译只是报告一个警告: 
       note: expected ‘long unsigned int *’ but argument is of type ‘long int **’
    2) 如果是在C++中, 以g++来编译, 将会得到一个错误:
       error: cannot convert ‘long int**’ to ‘long unsigned int*’ for argument ‘1’ to ‘void func_parameter_passing(long unsigned int*)’
> ### **5.2 clang** ###
    1) 如果在C语言中, 以clang来编译将得到一个警告:
       warning: incompatible pointer types passing 'int **' to parameter of type 'unsigned long *' [-Wincompatible-pointer-type]
       func_parameter_passing(&val);
                              ^~~~~
    2) 如果在C++中, 以clang++来编译将会得到一个错误提示:
       error: no matching function for call to 'func_parameter_passing'
       func_parameter_passing(&val);
       ^~~~~~~~~~~~~~~~~~~~~~
       note: candidate function not viable: no known conversion from 'int **' to 'unsigned long *' for 1st argument;
       func_parameter_passing(unsigned long* arg)
       ^
> ### **5.3 总结** ###
    1) 首先参数类型不匹配的情况本身就潜伏着未知的bug, 比如函数的参数类型是long*, 而传递进来的是一个short的地址的话, 而恰巧函数参数是输出的, 就会造成数据的丢失
    2) 在跨平台的项目中最好使用参数相匹配的变量
    3) 但在这个移植的项目中, 很多这种错误其实是没有错误的, 这里我们可以在调用函数的时候显示的转换一次即可




## **6. 创建目录** ##
        在windows下, 程序可以在任意地方创建目录, 并且所有用户具有任意读写权
    限。所以, 这里我们主要讨论在linux下创建目录:

> ### **6.1 创建带指定权限的目录** ###
    1) 直接使用mkdir函数即可
    2) 该函数的第二个参数便可指定你想创建的目录的权限
> ### **6.2 在其他目录创建指定权限的目录** ###
    1) 当你想在其他目录创建一个0777权限的目录时总是失败
       因为umask的默认值是0022, 所以你创建嘛目录权限为(0777 & ~0022)
    2) 所以在你需要在其他目录创建指定权限的目录时需要先使用umask(0)
       mode_t user_mode = xxx;
       umask(0);
       mkdir(dir_path, user_mode);



## **7. 未申明类型的static变量和const变量** ##
        // C++ 
        static SIZE_1 = 10;
        int main(int argc, char* argv[]) 
        {
          const SIZE_2 = 20;
          return 0;
        }
        在MSVC中一个static或者const未指定类型的变量会被默认指定为int类型, 而
    在gcc或clang中则会出错。  
        [注] 在MSVC的高版本中, 未指定类型的static或const变量会被指定为一个错
    误。

> ### **7.1 gcc** ###
    1) 在gcc中按照C语言的方式来编译(gcc -x c ...), 不会指定错误, 变量被当做int类型来处理
    2) 在gcc中按照C++的方式来编译(gcc -x c++ ...), 将会视为一个错误:
       error: ‘SIZE_1’ does not name a type
> ### **7.2 clang** ###
    1) 在clang中以C语言方式编译(clang -x c ...), 将报一个警告:
       warning: type specifier missing, defaults to 'int' [-Wimplicit-int]
       static SIZE_1 = 10;
       ~~~~~~ ^
    2) 在clang中以C++语言方式编译(clang -x c++ ...), 将被视为一个错误:
       error: C++ requires a type specifier for all declarations
       static SIZE_1 = 10;
       ~~~~~~ ^
> ### **7.3 结论** ###
    1) 指定任意一个变量的类型
    2) 不要为了偷懒而使用一些老式C(K & R)语言的风格, 请用高版本的编译器编译
    3) 为了程序的跨平台, 为了便于别人阅读代码, 请使用标准C/C++



## **8. offsetof** ##
        // C++ 
        struct A {
          int aa1;
          int aa2;
          int aa3;
        };
        struct B : public A {
          int bb1;
          int bb2;
        }; 
        static const int SIZE = offsetof(B, bb1);
> ### **8.1 问题说明** ###
    在程序中使用offsetof计算成员偏移量的时候(offsetof(B, bb1)), 在MSVC下  
    程序不会有警告和错误, 但是在gcc下会出现如下警告:  
        warning: invalid access to non-static data member ‘B::bb1’ of NULL object [-Winvalid-offsetof]
        warning: (perhaps the ‘offsetof’ macro was used incorrectly) [-Winvalid-offsetof]
    而在clang下则会出现如下警告:  
        warning: offset of no non-POD type 'B' [-Winvalid-offsetof]  
        static const int SIZE = offsetof(B, bb1);
                                ^~~~~~~~~~~~~~~~
        /usr/include/clang/3.0/include/stddef.h:52:24: note: expected from:
        #define offsetof(t, d) __builtin_offsetof(t, d)
                               ^
> ### **8.2 问题分析** ###
        根据ANSI C++的文档描述, offsetof宏仅限于POD联合和POD结构类型。
        POD(plain old data)就是C语言中的基本类型, 如: a built-in type, 
    pointer, union, struct, array, or class with a trivial constructor。
    其兼容C语言的struct, POD对象在进行复制的时候, 不必调用对象的复制构造
    函数或者operator=, 可以直接采用memcpy函数来提高效率。
        而NON-POD与POD相反, 可能是很复杂的类别或结构, 例如STL。
>> #### **8.2.1 POD类型(标量类型, C/C++基本类型)** ####
    1) 算术类型(arithmetic type)
    2) 枚举类型(enumeration type)
    3) 指针类型(pointer type)
    4) 指针到成员类型(pointer-to-member type)
    5) 整数类型(integral type)
    6) 浮点数类型(floating type)
    7) 有符号整数类型(signed char, short, int, long)
    8) 无符号整数类型(unsigned char, unsigned short, unsigned int, unsigned long)
    9) 字符类型char与宽字符类型wchar_t
    10) 布尔类型bool
    11) 浮点类型(float, double, long double)
    12) 空指针pointer-to-void(void*)
    13) 对象指针pointer-to-object与指向静态数据成员的指针pointer-to-static-member-data(都是行如T*, 其中T是对对象类型)
    14) 函数指针pointer-to-function与指向静态成员函数的指针pointer-to-static-member-function(都是形如T(*)(...), T是函数的返回值类型)
    15) 指针到非静态成员函数(pointer-to-nonstatic-member-function), 形如T(C::*)(...)表示指向类C的返回值类型为T的成员函数的指针

>> #### **8.2.2 POD类型(用户自定义类型/POD类类型)** ####
    1) 聚合类(POD-struct types)与聚合union(POD-union types), 且不具有以下成员:
       a) 指针到成员类型的非静态数据成员(包括数组)
       b) 非POD类类型的非静态数据成员(包括数组)
       c) 引用类型的(reference type)非静态数据成员
       d) 用户定义的拷贝和赋值算子
       e) 用户定义的析构函数
    2) 聚合实质任何的数组或类, 且不具有下述特征:
       a) 用户定义的析构函数
       b) 私有或保护的非静态数据成员
       c) 基类
       d) 虚函数
    3) POD类类型就是class, struct, union且不具有用户定义的构造函数, 析构
       函数, 拷贝算子, 赋值算子; 不具有继承关系, 没有基类; 不具有虚函数, 没
       有虚表; 非静态数据成员没有私有或保护属性的, 没有引用类型的, 没有非
       POD类类型的(及嵌套类都必须是POD), 没有指针到成员的类型(因为这个类型
       内含了this指针)

> ### **8.3 问题解决** ###
    自定义一个offsetof即可:
    #define offsetof_ex(s, m) \
        ((size_t)&(((s*)0x100)->m) - (size_t)0x100) 

> ### **8.4 附加技巧(如何判断一个类型为POD)** ###
    首先对POD类型做一个总结:
        POD类型必须满足以下条件: 将组成它的一个对象的各字节拷贝到一个字节数
    组中, 然后再将它们重新拷贝回原先的对象所占的存储区中, 此时对象应该仍具
    有它原来的值。
        POD类型: 标量类型, POD结构类型, POD联合类型, 以上这些类型的数组, 以
    及这些类型以const/volatile修饰的版本。
        POD结构: 一个聚合体, 其任何非静态成员的类型都不能是如下任何一种: 指
    向成员的指针, 非POD联合, 以及以上这些类型的数组或引用, 同时该聚合体类不
    允许包含用户自定义的拷贝赋值操作符和用户自定义的析构函数。
        POD类型的作用: POD类型允许C++与C交互!!!
    下面这个方法可以在编译期间就可以判断一个类型是否是POD类型, 如不是, 报错:
        // C++
        template <typename _Tp> struct must_be_pod {
          union {
            _Tp _Tp_is_POD_type;
          };
        };



## **9. 获取磁盘剩余空间** ##
> ### **9.1 Windows下** ###
        在Windows平台需要获取磁盘剩余空间可以使用GetDiskFreeSpace, 其具体使
    用请参见MSDN文档:
        BOOL WINAPI GetDiskFreeSpace(
          _In_  LPCTSTR lpRootPathName, 
          _Out_ LPDWORD lpSectorsPerCluster, 
          _Out_ LPDWORD lpBytesPerSector, 
          _Out_ LPDWORD lpNumberOfFreeClusters, 
          _Out_ LPDWORD lpTotalNumberOfClusters
        );
> ### **9.2 Linux下** ###
        在Linux平台下, 可以使用statfs来获取磁盘使用情况, 其具体请参见文档, 
    这里只简单列出其定义:
        #include <sys/vfs.h>    /* or <sys/statfs.h> */
        int statfs(const char* path, struct statfs* buf);
>
        #if __WORDSIZE == 32 
        # define __SWORD_TYPE   int
        #else /* __WORDSIZE == 64 */
        # define __SWORD_TYPE   long int 
        #endif
        struct statfs {
          __SWORD_TYPE f_type;      /* 文件系统类型 */
          __SWORD_TYPE f_bsize;     /* 经过优化的传输快大小 */
          fsblkcnt_t   f_blocks;    /* 文件系统数据块总数 */
          fsblkcnt_t   f_bfree;     /* 可用的块数 */
          fsblkcnt_t   f_bavail;    /* 非超级用户可获取的块数 */
          fsfilcnt_t   f_files;     /* 文件结点总数 */
          fsfilcnt_t   f_ffree;     /* 可用文件结点数 */
          fsid_t       f_fsid;      /* 文件系统标识 */
          __SWORD_TYPE f_namelen;   /* 文件名的最大长度 */
          __SWORD_TYPE f_frsize;
          __SWORD_TYPE f_spare[5];
        };



## **10. 调用外部程序** ##
        在程序中, 我们经常需要调用外部的程序来协助完成一些功能, 这时候就需要
    在程序中调用外部的程序, 这里简单介绍在Windows和Linux下的不同方式。
> ### **10.1 Windows下** ###
        在Windows平台下, 我们可以使用C库中的system来调用外部的程序, 也可以使
    用CreateProcess来调用外部的程序, 一般使用CreateProcess可供程序员选用的参
    数更多些, 也更灵活一些, 所以在项目中一般可以使用CreateProcess。
        相信, 大家对CreateProcess都比较熟悉的, 具体使用请参见MSDN:
        BOOL WINAPI CreateProcess(
          _In_opt_    LPCTSTR lpApplicationName, 
          _Inout_opt_ LPTSTR lpCommandLine, 
          _In_opt_    LPSECURITY_ATTRIBUTES lpProcessAttributes, 
          _In_opt_    LPSECURITY_ATTRIBUTES lpThreadAttributes, 
          _In_        BOOL bInheritHandles, 
          _In_        DWORD dwCreationFlags, 
          _In_opt_    LPVOID lpEnvironment, 
          _In_opt_    LPCTSTR lpCurrentDirectory, 
          _In_        LPSTARTUPINFO lpStartupInfo, 
          _Out_       LPPROCESS_INFORMATION lpProcessInformation
        );
> ### **10.2 Linux下** ###
        下面我们主要来讲解Linux下, 如何在程序中调用外部的程序, 同样的在linux
    下可以使用C库中的system来调用外部的程序; 可以建立管道来调用外部的命令; 
    也可以使用fork等创建一个子进程, 然后调用exec族的函数来调用外部程序。
    1) 使用system函数
        #include <stdlib.h>
        int system(const char* command);
> 
            system会调用fork产生子进程, 有子进行来调用/bin/sh -c来执行参数
        command所代表的命令, 此命令执行完成后随即返回原调用的进程, 在调用它
        的期间SIGCHLD信号会被暂时搁置, SIGINT和SIGQUIT信号则会被忽略。
            在编写具有SUID/SGID权限的程序时请不要使用system, 它会继承环境变
        量, 通过环境变量可能会造成系统安全的问题。
    2) popen(建立管道I/O)
        #include <stdio.h>
        FILE* popen(const char* command, const char* type);
        int pclose(FILE* stream);
> 
            popen会调用fork产生子进程, 然后子进程中调用/bin/sh -c来执行参数
        command的指令, 参数type可以使用"r"代表读取, "w"代表写入。根据type值
        popen会建立管道连接到子进程的标准输出设备或标准输入设备, 然后返回一
        个文件指针。随后进程就可以利用此文件指针来读取子进程的输出设备或写入
        到子进程的标准输出设备。
            在编写具有SUID/SGID权限的程序时请不要使用popen, popen会继承环境
        变量, 通过环境变量可能会造成系统安全的问题。
            例子:
> 
            char buf[128];
            FILE* fp = popen("cat /etc/passwd", "r");
            fgets(buf, sizeof(buf), fp);
            pclose(fp);
    3) 使用vfork创建子进程, 然后调用exec函数族
        #include <sys/types.h>
        #include <unistd.h>
        pid_t vfork(void);
> 
        先介绍进程的4要素:
            => 要有一段程序供该进程运行, 该程序可以被多个进程共享
            => 要有"私有财产", 即是进程专用的系统堆栈空间
            => 要有"户口", 即进程控制块, 具体实现是task_struct
            => 有独立的存储空间
            vfork与fork不同的地方在于, vfork创建出来的不是真正意义上的进程, 
        而是一个线程, 它却上独立的存储空间。
> 
        例子:
        char* argv[] = {"ls", "-la", "/home/", NULL};
        if (0 == vfork()) 
          execv("/bin/ls", argv);
        else 
          fprintf(stdout, "Parent process ...\n");




## **11. STL中的erase** ##
        // C++
        std::map<int, std::string> map_test;
        std::map<int, std::string>::iterator it = map_test.find(key);
        map_test.erase(it);
        我们知道在STL容器中, 根据erase来删除容器中的一个节点的时候, 当只是删
    除单个节点的时候, 不会出现任何问题; 但是在一个循环中使用的时候, 往往会被
    误用, 那是因为没有正确理解iterator的概念造成的。
        我们经常看见会被误用成如下:
        // C++
        for (it = map_test.begin(); it != map_test.end(); ++it) {
          printf("{key=>%d, val=>%s}\n", it->first, it->second.c_str());
          map_test.erase(it);
        }
        这种错误的写法会导致程序的行为不可知, 对于关联容器而言, 如果一个元素
    已经被删除了, 那么对应的迭代器就失效了, 不应该再被使用了, 否则会导致程序
    不可知的行为。
> ### **11.1 MSVC下** ###
        由于MSVC是使用的是P.J.Plauger STL, 而该实现的STL关联容器中, 如set, 
    multiset,map, multimap, hash_set, hash_multiset, hash_map, hash_multimap
    中的erase均返回了下一个元素的迭代器。
        因此, 在该版本的实现的STL可以使用如下方法(以上面map为例子):
        // C++
        for (it = map_test.begin(); it != map_test.end(); ) {
          if (key == it->first) {
            fprintf(stdout, "earase value: %s\n", it->second.c_str());
            it = map_test.erase(it);
          }
          else 
            ++it;
        }
> ### **11.2 GCC下** ###
        GCC中的STL使用的是SGI STL, 主要由STL之父Alexandar Stepanov实现, 在该
    实现版本中set, multiset, map, multimap, hash_set, hash_multiset, 
    hash_map, hash_multimap中erase的实现均是返回值是void。
        因此, 在使用该版本的STL实现的时候, 使用删除之前的迭代器定位下一个元
    素, 例子如下(以上面的map为例子):
        // C++
        for (it = map_test.begin(); it != map_test.end(); ++it) {
          if (key == it->first) {
            fprintf(stdout, "earase value: %s\n", it->second.c_str());
            map_test.erase(it++);
          }
        }
> ### **11.3 结论** ###
        在使用STL的时候请注意你所使用的STL实现版本, 不同的STL实现可能存在一
    些细微的区别, 但就是这些细微的差别如果不注意却可能造成程序行为不可预知, 
    所以我们在使用的时候应该多加注意。
        主流的MSVC和GCC/Clang的STL实现中的erase的不同如上面介绍, 顺序容器的
    erase都是返回了下一个元素的迭代器。



## **12. `__if_exists`和`__if_not_exists`关键字** ##
        这两个关键字主要用于判断传入模板类实现了什么方法, 包含了什么成员, 请
    看看下面的例子:
        // C++ 
        template <typename _Tp>
        class Hey : public _Tp {
        public:
          Hey(void) 
          {
            __if_exists(_Tp::sayHello) {
              // _Tp类支持sayHello()
              sayHello();
            }
          }
        };
        __if_exists和__if_not_exists语法意义, 只是意义相反。
> ### **12.1 使用时需要注意** ###
    1) 它们是条件判断语句, 不是预编译语句, 请不要把他们当中#ifdef ... #endif
       来使用
    2) 只能判断符号是否存在, 对于函数, 不能判断某种特定函数形式是否存在
    3) 大括号不能省略
    4) 程序必须使用/EHsc, 即打开C++异常处理
    5) 只在MSVC中特有的关键字
> ### **12.2 GCC/Clang** ###
        由于这两个关键字在gcc/Clang并不存在代替的关键字, 所以移植到Linux平台
    的时候, 遇到使用了这两个关键字判断的模板, 需要将模板特化, 根据使用了的类
    行, 单独写对应的函数实现或类实现。
> ### **12.3 结论** ###
    1) 在开发跨平台的程序的时候, 建议不要使用类似的MSVC的特有关键字
    2) 另一种方法是, 在Linux环境下, 将使用了这两个关键字的函数或类特化



## **13. 在一命名空间中访问全局函数** ##
        现在需要在一命名空间中的函数或类中范围一个命名空间外的外部函数, 但是
    该函数是在命名空间值后才实现的, 如下:
        // C++
        namespace g {
        class GlobalFunction {
        public:
          void show(void) 
          {
            void global_show(void);
            global_show();
          }
        };
        }
        void 
        global_show(void) 
        {
          fprintf(stdout, "Testing global function ...\n");
        }
> ### **13.1 MSVC中** ###
        上面的代码在MSVC中编译的时候可以安全通过, 编译器在链接的时候会先去找
    命名空间g中有没有global_show的实现, 如果有则调用g::global_show; 如果命名
    空间g中没有global_show的实现才会去调用外部的全局函数global_show函数。
        而在g::GlobalFunction::show函数中的函数声明(void global_show(void))
    也是如此, 默认其是命名空间g中的函数, 当命名空间中没有此函数的时候则表示
    其是全局函数。
        因此上面的代码在MSVC中是正确的。
> ### **13.2 GCC/Clang中** ###
        上面的代码在GCC或者Clang中编译的时候会得到一个链接警告, 编译器在链接
    的时候会去寻找命名空间中是否有函数global_show, 结果找不到则报出一个链接
    错误。
        在函数g::GlobalFunction::show中的声明(void global_show(void))则是表
    示该函数是命名空间g中函数, 所以链接的时候也只会寻找命名空间中是否有函数
    global_show。因此上面的代码在GCC或Clang中编译的时候会得到一个链接错误。
> ### **13.3 结论** ###
      1) 上面的代码在GCC/Clang下, 可以先在namespace前声明一个函数原型, 这样
         在g::GlobalFunction::show中就不需要再声明了, 那么链接的时候, 编译器
         寻找是否有命名空间g中的函数global_show。有, 则调用g::global_show,
         如果没有则调用全局空间的global_show函数。
      2) 建议在编写代码的时候不要使用这种老式的C风格
      3) 所有需要调用的函数要么都给一个声明, 要么都在调用之前实现
> ### **13.4 修改** ###
        上面的代码, 考虑到全局函数可能也会调用命名空间中的某些函数或使用命名
    空间中的变量等, 我们仍旧保持函数实现在namespace后面, 其修改如下:
        // C++
        void global_show(void);
        namespace g {
        class GlobalFunction {
        public:
          void show(void) 
          {
            global_show();
          }
        };
        }
        void 
        global_show(void) 
        {
          fprintf(stdout, "Testing global function ...\n");
        }




## **14. 内存对齐** ##
        项目中经常需要遇到结构对齐的问题, 在Windows下可以在MSVC中全局的设置
    项目中所以结构的对齐格式, 抑或使用#pragma pack指令来对需要的对齐的结构设
    置结构对齐; 同样的在Linux下也可以使用GCC的关键字#pragma pack来设置结构对
    齐, 或者使用GNU的扩展__attribute__((aligned (n)))来设置对齐。
        但是, GCC中使用#pragma pack对齐, GCC依然是使用4字节对齐的, 即使设置
    了高于4对齐的, 也会按照4字节对齐方式。
        MSVC默认是按照8字节对齐, GCC是按照8字节对齐 。。。
> ### **14.1 内存对齐原因** ###
    1) 平台原因:
       并非所有的硬件平台都能够访问任意地址上的任意数据, 某些硬件平台只能够
       在某些地址处获取某些特定类型的数据, 否则会抛出硬件异常
    2) 性能原因:
       数据结构(尤其是栈)应该尽可能的在自然边界上对齐, 原因在于, 为了访问未
       对齐的内存, 处理器需要作两次内存访问, 而对齐的内存访问仅仅需要一次访
       问即可
> ### **14.2 内存对齐规则** ###
        每个特定平台上的编译器都有自己默认的"对齐系数"(对齐模数), 可以通过预
    编译命令#pragma pack(n), n = 1, 2, 4, 8, 16来改变这一系数, 其中n就是你要
    指定的对齐系数。
    1) 数据成员对齐规则: 
       结构或联合的数据成员, 第一个数据成员放在offset为0的地方, 以后每个数据
       成员的对齐按照#pragma pack指定的数值和这个数据成员自身长度中比较小的
       那个进行
    2) 结构或联合的整体对齐规则:
       在数据成员完成各自对齐之后, 结构或联合本身也要进行对齐, 对齐按照预编
       译指令#pragma pack指定的数值和结构或联合最大数据数据成员长度中, 比较
       小的那个进行
    3) 当#pragma pack的n值等于或超过所有数据成员长度的时候, 这个n值将不产生
       任何效果和作用
> ### **14.3 更改编译器默认字节对齐的方式** ###
    1) #pragma pack(n)
       该预编译指令在GCC(4.x以上的版本)和MSVC中均有作用, 对齐的规则都是按照
       上面的规则来进行的, 但是在GCC中当n > 4的时候, 编译器也只会按照4字节来
       对齐的
    2) MSVC中使用/Zp编译选项
       使用MSVC编译项目的情况下, 可以使用/Zp编译选项对整个项目设置字节对齐, 
       /Zp[[alignment]] alignment的取值为1, 2, 4, 8, 16
    3) GCC中-fpack-struct
       如果需要结构按照其成员实际大小排列的时候, 可以是使用-fpack-struct选项
       即是将结构按1字节对齐, 替换掉GCC默认的4字节对齐
    4) GCC中使用__attribute__((packed))
       取消结构在编译过程中的优化对齐, 按照实际占用的字节数进行对齐
    5) GCC中使用__attribute__((aligned (n)))
       用在需要使用自定义结构对齐的结构尾巴上, 让整个结构对齐在n字节的自然边
       界上, 成员总是按照自身宽度对齐。
           当n小于最大成员的字节数时, 整个结构按照结构中的最大成员宽度对齐, 
       前提是结构中最大的数据成员是4字节, 当最大成员大于4字节时, 也会按照4字
       节来对齐(GCC默认是按照4字节对齐的);
           当n大于最大成员的字节数时, 整个结构按照n对齐;
> ### **14.4 结论** ###
    1) 在项目中最好对结构体指定一个对齐方式, 避免使用编译器默认的结构对齐
    2) 在跨平台的项目上最好不要使用#pragma pack指定8字节对齐(因为在GCC中, 即
       使指定的对齐值大于4也只会按照4字节对齐)
    3) 最好不要混合使用__attribute__((aligned (n)))和#pragma pack(n), 因为它
       们对整个结构体(或联合体)的对齐方式是不一样的
    4) 在跨平台的项目中也不要使用MSVC的/Zp编译选项来设置字节对齐, 因为在GCC
       中不存在(或许是我自己没找到???)设置整个工程的字节对齐方式(特别是对齐
       字节大于4的情况)
