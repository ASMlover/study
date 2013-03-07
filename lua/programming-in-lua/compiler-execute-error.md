# **编译, 执行与错误** #
***


## **1. 编译** ##
    1) dofile用于运行lua代码, 实际dofile只是一个辅助函数, loadfile才会真正的
       核心工作
    2) loadfile会从一个文件加载lua代码, 但不会运行代码, 只会编译代码, 然后将
       编译的结果作为一个函数返回; loadfile不会引发错误, 只会返回错误值并不
       处理错误
       -- lua
       function dofile(filename)
        local f = assert(loadfile(filename))
        return f()
       end 
    3) 如果需多次运行一个文件, 那么只需在调用一次loadfile后, 多次调用它的返
       回结果就可以了, 相对多次调用dofile来说, 由于只编译一次文件, 开销就会
       小很多
    4) loadstring与loadfile类似, 只不过它是从一个字符串中读取代码
    5) 如果代码中有语法错误, loadstring就会返回nil, 为了清楚显示错误, 可以使
       用assert
    6) loadstring最典型的用处就是执行外部代码, 就是那些位于程序之外的代码
    7) loadstring期望的是输入一个程序块, 即是一系列语句
    8) loadfile和loadstring都是基于原始函数load的, loadfile和loadstring分别
       从文件和字符串中读取程序块, load则接收一个读取器函数, 并在内部调用它
       来获取程序块
    9) lua将所有独立的程序块视为一个匿名函数的函数体, 并且该匿名函数还具有可
       变长实参
    10) 这些函数不会带来任何副作用, 只是将程序块编译为一种中间表示, 然后将结
       果作为一个匿名函数来返回


## **2. C代码** ##
    1) lua提供的所有关于动态链接的功能都在一个函数中(package.loadlib), 该函
       数有两个字符串: 动态库的完整路径和一个函数名称, 类如:
       local path = "/usr/local/lib/lua/5.1/socket.so"
       local f = package.loadlib(path, "luaopen_socket")
    2) loadlib加载指定的库, 并将其链接如lua; 其冰没有调用库中的任何函数, 只
       是将C函数作为一个lua函数返回
    3) loadlib必须提供库的完整路径及正确的函数名称
    4) 通常使用require来加载C程序库, 这个函数会搜索指定的库, 然后使用loadlib
       来加载库, 并返回初始化函数


## **3. 错误** ##
    1) lua所遇到的任何未预期条件都会引发一个错误
    2) assert检查其第一个参数是否为true, 若为true则简单的返回该参数, 否则就
       引发一个错误; 它第二个参数是一个可选的信息字符串
    3) 当一个函数遭遇异常时, 可以采取两种基本的行为: 返回错误代码或引发一个
       错误
    4) 可以在调用函数的时候使用assert, 如果失败就会引发一个错误


## **4. 错误处理与异常** ##
    1) 如果需要在lua中处理错误, 则必须使用函数pcall来包装需要执行的代码
    2) pcall函数会以一种"保护模式"来调用它的第一个参数, 因此pcall可以捕获含
       数执行中的错误; 如果没有发生错误, pcall会返回true及函数调用的返回值; 
       否则, 返回false及错误代码


## **5. 错误消息与追溯** ##
    1) error函数有第二个附加参数level, 用于指出应由调用层级中的哪个函数来报
       告当前的错误, 也就是说明了谁应该为错误负责
    2) debug.debug提供一个lua提示符, 让用户来检查错误的原因
    3) debug.traceback根据调用栈来构建一个扩展的错误消息
    4) 可在任何时候调用debug.traceback函数来获取当前执行的调用栈
       print(debug.traceback())
