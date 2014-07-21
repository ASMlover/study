# **Lua环境**
***



## **解释器程序**
    1) 程序第一行以#开头, 加载时解释器会忽略该行
        #!/usr/bin/lua 
        #!/usr/bin/env lua 
    2) 解释器用法:
        lua [选项参数] [脚本[参数]]
        * 选项参数'-e': 可直接在命令行中输入代码
        * 选项参数'-l': 用于加载库文件



## **编译和错误**
    1) 编译
        * dofile用于运行Lua代码块
        * loadfile从文件加载Lua代码块, 编译代码然后将编译结构作为一函数返回
        * loadstring与loadfile类似
    2) C 代码
        * package.loadlib加载指定的库, 并将其链接入Lua;
        * package.loadlib必须提供库完整路径以及正确函数名称;
    3) assert




## **环境**
    Lua所有的全局变量都保存在一个常规的table中, 这个table就是'环境';
      * 不需要再为全局变量创造一种数据结构, 简化了内部实现;
      * 可以像操作其他table一样来操作该table;
    Lua环境保存在_G中;
    1) Lua中全局变量不需要声明就可以直接使用;
       检查全局变量_G中不存在的变量:
          setmetatable(_G, {
            __newindex = function(_, n)
              error('attempt to write to undeclared variable'..n, 2)
            end, 
            __index = function(_, n)
              error('attempt to read undeclared variable'..n, 2)
            end, 
          })
    2) 绕过元表声明新变量的方法(使用rawset)
          function declare(name, initval)
            rawset(_G, name, initval or false)
          end
    3) debug.getinfo(2, 'S')返回一个table, 它的what字段表示调用元方法的函数
       是主程序块还是普通的Lua函数, 又或者是C函数;
    4) 可以通过setfenv来改变一个函数的环境, 参数是一个函数和一个崭新的环境
       table; 第一个参数除了可以指定为函数本身, 还可以指定为一个数字, 用来
       表示当前函数调用栈的层数(1表示当前函数, 2表示调用当前函数的函数 ...)
