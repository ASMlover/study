# **线程和状态** #
***

        Lua不支持真正的多线程; 首先ANSI C没有提供这样的功能, 并且也没有可移
    植的方法能在Lua中实现这种机制; 其次在Lua中引入多线程并不是一个好的选择.


## **1. 多个线程** ##
    1) 一个线程本质上是一个协同程序
    2) 调用lua_newthread就可以在一个状态中创建其他的线程:
       lua_State* lua_newthread(lua_State* L);
       该函数返回一个lua_State指针, 表示新建的线程; 还会降新线程作为一个类型
       为"thread"的值压入栈中
    3) 每个线程都有其自己的栈
    4) lua_xmove可以在两个栈之间移动lua值
       lua_xmove(F, T, n)会从栈F中弹出n个元素, 并将它们压入T中 
    5) lua_resume可以启动一个协同程序, 将待调用的函数压入栈中, 并压入其参数,
       最后在调用lua_resume的时候传入参数的数量narg
       int lua_resume(lua_State* L, int narg);


## **2. Lua状态** ##
    1) 调用luaL_newstate会创建一个新的Lua状态
    2) 不同lua状态是各自完全独立的, 它们之间不共享任何数据
