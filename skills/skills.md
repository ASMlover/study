# **Skills(小技巧)** #
***
    主要记录了各种编程的小技巧或者小知识点


## **1. 函数调用约定(MSVC)** ##
        int add(int a,  int b)  
        {  
          return a + b;  
        }  

        函数调用约定是, 主调和被调函数之间的协议, 约定函数参数以什么顺序依次
    压栈, 以及函数结束后由谁来完成对入栈参数的清理(清栈)
> ### **1.1 __cdecl调用约定** ###
    1) 函数的参数从右到左依次压栈
    2) 函数的调用者在push参数, 调用add之后(add esp 8), 来平衡栈帧
    3) 由函数的调用者来平衡栈帧
    4) 函数中只有简单的(ret 0), 将清栈的操作交由函数的调用者来完成 

> ### **1.2 __stdcall调用约定** ###
    1) 函数的参数从右到左依次压栈
    2) 函数的调用者只是在push参数后直接盗用add
    3) 由函数自己类平衡栈帧
    4) 函数体自身有(ret 8)来平衡栈帧

> ### **1.3 具体的流程请查看例子** ###
    1) 以__cdecl调用约定编译后的函数为_add
    2) 以__stdcall调用约定编译后的函数为_add@8 (_函数名@参数的总字节数)
    3) 参见例子 ./call-convention/main.c 



## **2. 编译时候指定任意后缀的文件** ##
> ### **2.1 引子** ###
        查看STLport的源代码的时候发现有.c后缀的文件, 单文件中使用的是C++语法
    , 感到很奇怪。后来一想, 其实文件的后缀在*nix系统下是没有意义的, 所以猜想
    编译器定有什么编译选项可以指定任意格式的文件。

> ### **2.2 内幕** ###
    1) MSVC下有个编译选项/Tp, 指定文件按C++编译
    2) MSVC下有个编译选项/Tc, 指定文件按C编译
    3) GCC下有个编译选项 -x language, 可以在language指定具体文件 



## **3. git配置** ##
> ### **3.1 配置颜色** ###
    1) 常用的颜色:  
       git config --global color.branch auto  
       git config --global color.diff auto  
       git config --global color.interactive auto  
       git config --global color.status auto  
    2) 把颜色全部都打开: 
       git config --global color.ui true
> ### **3.2 编辑器配置** ###
    git config --global core.editor 'vim'
> ### **3.3 配置用户信息** ###
    git config --global user.name "your name"
    git config --global user.email you@example.com
> ### **3.4 一个git配置例子** ###
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
   

## **4. 好玩儿** ##
> ### **4.1 一行代码将浏览器变为记事本** ###
    1) 在浏览器地址栏输入: 
       data:text/html, <html contenteditable>
    2) 由这一行代码引起的头脑风暴, [请查看](http://www.oschina.net/news/37360/one-line-code-online-editor)



## **5. Windows路径转换** ##
> ### **5.1 在命令行下从一个地方转到另一个地方** ###
        假定当前所在的位置是(C:\>), 现在需要直接转换到其他地方, 如果是在同一
    个驱动器下, 直接cd转换即可; 但是当在其他的驱动器(D:\demo\)的时候, 我们需
    要使用其他方法:
        1) C:\> cd /d D:\demo 
        2) pushd D:\demo 
        3) 方法1是直接跳转, 在返回的时候不容易; 方法二跳转后可以使用popd返回
> ### **5.2 命令行下打开窗口目录** ###
        在命令行下有时需要进入图形窗口目录查看, 我们可以使用explorer命令来完
    成该任务, 假设当前位置是(C:\>), 我们要进入(D:\demo):
        C:\> explorer D:\demo 



## **6. 新安装的动态库在系统中找不到(Linux)** ##
        下面的例子以libzmq为例, 系统中新安装了libzmq的动态库到/usr/local/lib
    下, 但是在程序运行的时候却不能加载;
        可以使用 sudo /sbin/ldconfig -v | grep libzmq来检查系统, 如果没有出
    现下面的内容:
        libzmq.so.1 -> libzmq.so.1.0.1
        则需要在/etc/ld.so.conf文件的最后一行加上 /usr/local/lib/ 

        另外新安装的共享库, 找不到的情况下还需要运行sudo ldconfig来配置动态
    库的环境
