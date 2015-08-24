# **Skills(小技巧)**
***
    主要记录了各种编程的小技巧或者小知识点


## **1. 函数调用约定(MSVC)**
        int add(int a,  int b)  
        {  
          return a + b;  
        }  

        函数调用约定是, 主调和被调函数之间的协议, 约定函数参数以什么顺序依次
    压栈, 以及函数结束后由谁来完成对入栈参数的清理(清栈)
> ### **1.1 __cdecl调用约定**
    1) 函数的参数从右到左依次压栈
    2) 函数的调用者在push参数, 调用add之后(add esp 8), 来平衡栈帧
    3) 由函数的调用者来平衡栈帧
    4) 函数中只有简单的(ret 0), 将清栈的操作交由函数的调用者来完成 

> ### **1.2 __stdcall调用约定**
    1) 函数的参数从右到左依次压栈
    2) 函数的调用者只是在push参数后直接盗用add
    3) 由函数自己类平衡栈帧
    4) 函数体自身有(ret 8)来平衡栈帧

> ### **1.3 具体的流程请查看例子**
    1) 以__cdecl调用约定编译后的函数为_add
    2) 以__stdcall调用约定编译后的函数为_add@8 (_函数名@参数的总字节数)
    3) 参见例子 ./call-convention/main.c 



## **2. 编译时候指定任意后缀的文件**
> ### **2.1 引子**
        查看STLport的源代码的时候发现有.c后缀的文件, 单文件中使用的是C++语法
    , 感到很奇怪。后来一想, 其实文件的后缀在*nix系统下是没有意义的, 所以猜想
    编译器定有什么编译选项可以指定任意格式的文件。

> ### **2.2 内幕**
    1) MSVC下有个编译选项/Tp, 指定文件按C++编译
    2) MSVC下有个编译选项/Tc, 指定文件按C编译
    3) GCC下有个编译选项 -x language, 可以在language指定具体文件 



## **3. git配置**
> ### **3.1 配置颜色**
    1) 常用的颜色:  
       git config --global color.branch auto  
       git config --global color.diff auto  
       git config --global color.interactive auto  
       git config --global color.status auto  
    2) 把颜色全部都打开: 
       git config --global color.ui true
> ### **3.2 编辑器配置**
    git config --global core.editor 'vim'
> ### **3.3 配置用户信息** ###
    git config --global user.name "your name"
    git config --global user.email you@example.com
> ### **3.4 一个git配置例子**
    [color]
      ui = true
    [core]
      editor = vim
    [user]
      name = ASMlover
      email = asmlover@126.com
    [pretty]
      graph = %Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset
    [alias]
      lg = log --pretty=graph --graph --abbrev-commit --
   

## **4. 好玩儿**
> ### **4.1 一行代码将浏览器变为记事本**
    1) 在浏览器地址栏输入: 
       data:text/html, <html contenteditable>
    2) 由这一行代码引起的头脑风暴, [请查看](http://www.oschina.net/news/37360/one-line-code-online-editor)



## **5. Windows路径转换**
> ### **5.1 在命令行下从一个地方转到另一个地方**
        假定当前所在的位置是(C:\>), 现在需要直接转换到其他地方, 如果是在同一
    个驱动器下, 直接cd转换即可; 但是当在其他的驱动器(D:\demo\)的时候, 我们需
    要使用其他方法:
        1) C:\> cd /d D:\demo 
        2) pushd D:\demo 
        3) 方法1是直接跳转, 在返回的时候不容易; 方法二跳转后可以使用popd返回
> ### **5.2 命令行下打开窗口目录**
        在命令行下有时需要进入图形窗口目录查看, 我们可以使用explorer命令来完
    成该任务, 假设当前位置是(C:\>), 我们要进入(D:\demo):
        C:\> explorer D:\demo 
        另外一种方法是使用start命令(进入当前目录):
        C:\> start .



## **6. 新安装的动态库在系统中找不到(Linux)**
        下面的例子以libzmq为例, 系统中新安装了libzmq的动态库到/usr/local/lib
    下, 但是在程序运行的时候却不能加载;
        可以使用 sudo /sbin/ldconfig -v | grep libzmq来检查系统, 如果没有出
    现下面的内容:
        libzmq.so.1 -> libzmq.so.1.0.1
        则需要在/etc/ld.so.conf文件的最后一行加上 /usr/local/lib/ 

        另外新安装的共享库, 找不到的情况下还需要运行sudo ldconfig来配置动态
    库的环境




## **7. 玩家二进制数据升级的问题**
    当涉及玩家的二进制数据升级的时候, 可以对玩家的数据打上版本的标记, 升级的
    时候直接写两套代码分别处理不同版本的数据信息;
    这样就可以处理不管是在数据结构的尾部增加还是在中间增加的玩家数据了;



## **8. 网络库Message Queue设计**
    可以设计两个队列, 各个工作线程将读取的数据推送到一个队列, 然后逻辑主线程
    再从另外一个线程读取消息数据信息;
    读取的数据信息可以直接设计成buffer即可, 这样每次读取数据信息的时候直接根
    据读取位置读取数据信息即可, 这样就可以避免频繁的申请释放内存了;
    两个数据队列直接采用无锁队列即可;
    
    读取的队列无锁, 网络处理线程读取的数据加锁写入写队列, 两个队列互相交换;
    1. 读优先 - 监测到读队列为空, 翻转两个队列
    2. 写优先 - 写队列满了之后再翻转两个数据队列
    3. 如果读队列中的数据没有取完, 而写队列满的情况, 后续来的数据丢掉



## **9. MySQL datetime转换为time_t**
> ### **在MySQL中转换**
    SELECT UNIT_TIMESTAMP(`time`) FROM `table_name`;
> ### **C++中转换**
    其实就是字符串类型的time转换为time_t类型, 可以先转换成tm结构再转换成
    time_t的数值类型;
    struct tm tm_out;
    strptime(src_time_str, "%Y-%m-%d %H:%M:%s", &tm_out);
    time_t t = mktime(&tm_out);



## **10. g++编译C++11**
    需要加上编译选项-std=c++0x
> ### **常用的C++11特性**
    1. shared_ptr
        在<memory>头文件中, 在命名空间std下面;



## **11. 递归删除某类型的文件**
    我们以orig文件类型为例子
> ### **Windows**
    \> del /s /f *.orig
> ### **Linux**
    $ find . -name '*.orig' -type f -print -exec rm -rf {} \;
    1. ".":               表示从当前目录开始递归查找
    2. "-name '*.orig'":  查找所有是orig结尾的文件或文件夹
    3. "-type f":         查找的类型是文件
    4. "-print":          输出查找的文件目录名
    5. "-exec":           后面跟一个执行命令, 将find出的文件或目录执行该命令
                          然后再跟{}, 一个空格, 一个\, 最后是一个分号;



## **12. 64位数据**
    Windows下面__int64同Linux的long long有一个区别, 就是不支持位移操作;



## **13. 时间**
> ### **Windows**
    1. 获取频率
        BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
        从理论上说执行不成功返回FALSE, 但是对与Pentiums以前的机器只能是失败;
    2. 获取时间
        int64_t t;
        QueryPerformanceCounter((LARGE_INTEGER*)(&t));
        获取的是从系统启动之后的滴答数量, 用时间除以频率就是从系统其他之后过
        去的秒数:
        t = t / freq;
        如果想获取毫秒数, 将每秒滴答的频率转换为每毫秒滴答即可:
        freq =  freq / 1000;
        QueryPerformanceCounter((LARGE_INTEGER*)(&t));
        t = t /freq;
> ### **Linux**
    使用gettimeofday
    这个函数需要使用到<sys/time.h>和timeval结构;
    timeval结构的tv_sec是从1970年之后的秒数量, tv_usec是从当前秒数过去的微
    秒数
> ### **功能代码**
    uint64_t GetTimeMS(void) {
    #ifdef WIN_PLATFORM
      uint64_t t;
      QueryPerformanceCounter((LARGE_INTEGER*)(&t));
      return t / freq;
    #else
      struct timeval t;
      uint64_t s;
      gettimeofday(&t, 0);
      s = t.tv_sec;
      s *= 1000;
      s += (t.tv_usec / 1000);
      return s;
    #endif
    }


## **14. 类型转换**
    template <typename T>
    inline std::string ToString(const T& v) {
      std::stringstream s;
      s << v;
      return s.str();
    }

    template <typename T>
    inline T ToType(const std::string& v) {
      std::stringstream s;
      s << v;
      T t;
      s >> t;
      return t;
    }



## **15. VT100控制码**
    [0m     复位所有的颜色和文本属性
    [1m     明亮/粗体颜色
    [2m     暗淡/取消粗体颜色
    [4m     加下划线文本
    [5m     闪烁文本
    [7m     反转颜色文本(交换前景和背景颜色)
    [8m     隐藏文本(不显示字符)
    [30m    黑色前景颜色
    [31m    红色前景颜色
    [32m    绿色前景颜色
    [33m    黄色前景颜色
    [34m    蓝色前景颜色
    [35m    品红色前景颜色
    [36m    青色前景颜色
    [37m    白色前景颜色
    [40m    黑色背景颜色
    [41m    红色背景颜色
    [42m    绿色背景颜色 
    [43m    黄色背景颜色
    [44m    蓝色背景颜色 
    [45m    品红色背景颜色
    [46m    青色背景颜色
    [47m    白色背景颜色
    [<R>;<C>H   将光标移到<R>行<C>列, 如果<R><C>省略, 则移动光标到原来位置
    [<C>A   光标向上移动<C>行, 如<C>省略, 向上移动一行
    [<C>B   光标向下移动<C>行, 如<C>省略, 向下移动一行
    [<C>C   光标向前移动<C>个空格, 如果<C>省略, 之移动一个空格
    [K      清除当前行中光标后面的所有内容
    [1K     清除当前行中光标前面的所有内容
    [2K     清除当前行 
    [J      清除当前行下面的每一行
    [1J     清除当前行上面的每一行
    [2J     清除整个屏幕


## **16. 函数栈空间的大小**
    C/C++语言函数的栈空间是有限制的; Windows一般默认是1M; 
    linux下查看栈空间大小:
      $ ulimit -a



## **17. GCC升级到4.8**
    $ sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    $ sudo apt-get update
    $ sudo apt-get install gcc-4.8
    $ sudo apt-get install g++-4.8
    $ cd /usr/bin/ 
    $ sudo rm g++
    $ sudo rm gcc 
    $ sudo ln -s gcc-4.8 gcc 
    $ sudo ln -s g++-4.8 g++


## **18. VS恢复默认设置和默认Layout**
		工具->导入和导出设置->重置所有设置


## **19. FOIL二项式乘法**
    (a + b)(c + d) => 
    * First   (a + b)(c + d)
               |______|
    * Outer   (a + b)(c + d)
               |__________|
    * Inner   (a + b)(c + d)
                   |__|
    * Last    (a + b)(c + d)
                   |______|

## **20. Python logging**
        logger = logging.getLogger(logger_name)
        logging.basicConfig(level=logging.INFO)
        logger.info(logging_infomation)

## **21. lua skill**
> ### **1) string**
    1) 内部有一个hash table来存储字符串
    2) 对每一个数字进行hash得到一个key
    3) 字符串只读，修改的时候会创建一个新字符串
    4) 频繁修改字符串会导致频繁的hash，很耗时
    5) 尽量不要新构建字符，多个字符要构成一个大字符的时候用table.concat
> ### **2) table**
    1) 有两个域，数组域和hash域
    2) #取数组长度，但不是len方法
    3) #是二分法查找最后一个非nil的元素的索引
    4) 数组最好使用table.insert和table.remove

## **22. svn技巧**
    1) 删除带有@文件名的文件(在文件名尾巴加一个@即可)
        svn rm --force "Default-568h@2x.png"@

## **23. SecureCRT密钥转为Putty密钥**
    1) 在SecureCRT
        工具 -> 转换私钥为OpenSSH格式
    2) 打开puttygen.exe
        转换 -> 导入密钥 -> 生成 -> 保存私钥

## **24. Boost相关**
> ### **24.1 Boost编译**
    1) 打开VS2013开发人员命令提示
    2) 进入boost目录
    3) \> bootstrap.bat
    4) \> bjam.exe stage --toolset=msvc-12.0 --stagedir=".\stage\x86" 
          link=static runtime-link=shared threading=multi debug release
    5) \> bjam.exe stage --toolset=msvc-12.0 --stagedir=".\stage\x64" 
          link=static runtime-link=shared threading=multi debug release
          address-model=64
    6) bjam参数说明：
        * stage/install: stage只生成(dll/lib), install还会生成包含头文件的
                         include目录；
        * toolset: 指定编译器(gcc, clang, msvc-12.0)；
        * without/with: 选择不编译/编译哪些库，默认全编译；
        * stagedir/prefix: stage时使用stagedir，install时使用prefix，编译生
                           成文件的路径；
        * build-dir: 编译生成的中间文件的路径；默认是bin.v2；
        * link: 生成动态链接库/静态链接库；动态库需要使用shared方式；
        * runtime-link: 动态/静态链接C/C++运行时库，也有shared和static两种方
                        式；
        * threading: 单/多线程编译(multi/single)；
        * debug/release: 编译debug/release版本；
> ### **24.2 Boost裁剪**
    1) 进入tools\bcp目录
    2) \> ..\..\bjam
    3) 回到boost目录
    4) \> .\dist\bin\bcp boost\regex.hpp literegex
    5) 生成的是一个裁剪后的regex
