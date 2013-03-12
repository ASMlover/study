# **调试库** #
***


## **1. 自省机制** ##
    1) 主要的自省函数时debug.getinfo; 第一个参数是一个函数或一个栈层
    2) 当为某函数foo调用debug.getinfo(foo), 会得到一个table; table字段如下:
        source -> 函数定义的位置
        short_src -> source的短版本(最多60个字符), 可用于错误信息中
        linedefined -> 该函数定义在源代码中的第一行的行号
        lastlinedefined -> 该函数定义在源代码中的最后一行的行号
        what -> 函数类型("lua"->Lua函数, "C"->C函数, "main"->lua主程序块)
        name -> 该函数的一个适当名称
        namewhat -> 上一个字段的含义(可能是"global", "local", "method", 
                    "field"和""), 空字符串表示Lua没有找到该函数的名称
        nups -> 该函数的upvalue的数量
        activelines -> 一个table, 包含了该函数的所有活动行的集合
        func -> 函数本身
    3) 当foo是一个C函数时, 只有字段what, name和namewhat有意义
    4) 用一个数字<n>调用debug.getinfo(n), 可以得到相应栈层上函数的数据; 
       <n>是1, 可以得到调用debug.getinfo的那个函数的数据;
       <n>大雨栈中函数的总数, debug.getinfo返回nil;
       当用数字来调用debug.getinfo时, table中还会包含currrentline字段, 表示
       此时这个函数正在执行的那行;
    5) getinfo的第二个参数(可选), 用于指定希望获取那些信息
        'n'       选择name和namewhat
        'f'       选择func
        'S'       选择source, short_src, what, linedefined, lastlinedefined
        'l'       选择currrentline
        'L'       选择activelines
        'u'       选择nups
> ### **1.1 访问局部变量** ###
    1) 可用debug.getlocal来检查任意活动函数的局部变量
    2) 该函数有两个参数: 一个时希望查询的函数栈层, 另一个是变量的索引
    3) 该函数返回变量的名字和它当前的值
    4) 如果变量索引大雨活动变量的总数, 返回nil
    5) 如果栈层无效, 会引发一个错误
    6) debug.setlocal改变局部变量的值, 前两个参数与getlocal相同, 第3个参数是
       该变量的新值; 返回变量名, 如果变量索引超出返回, 返回nil
> ### **1.2 访问非局部的变量** ###
    1) getupvalue, 可以访问为一个lua函数所使用的"非局部的变量";
       第一个参数是一个函数(是一个closure);
       第二个参数是变量索引
    2) debug.setupvalue, 修改"非局部的变量";
       第一个参数是closure;
       第二个参数是一个变量索引;
       第三个参数是一个新值
> ### **1.2 访问其他协同程序** ###
    1) 调试库所有自省函数都接受一个可选的协同程序参数作为第一个参数, 这样就
       可以从外部来检查这个协同程序



## **2. 钩子** ##
    1) 允许用户注册一个钩子函数, 这个函数会在程序运行中的某个特定时间发生时
       被调用
    2) 触发钩子的事件:
        每当lua调用一个函数时产生的call事件;
        每当函数返回时产生的return事件;
        每当lua开始执行一行新代码时产生的line事件;
        当执行完指定数量的指令后产生的count事件;
    3) 用一个字符串参数来调用钩子函数, 该字符串描述了导致调用钩子的事件
        "call", "return", "line"或"count"
    4) 对line事件, 还可以传入第二个参数, 表示代码的行号
    5) debug.sethook
        第一个参数是钩子函数;
        第二个参数是一个字符串, 描述了需要监控的事件;
        第三个参数是个可选数字, 说明多久获得一次count事件
