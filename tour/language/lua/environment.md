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
