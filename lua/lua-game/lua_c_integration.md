# **Lua与C/C++整合** #
***


## **1. lua环境** ##
        由所有可操作的数据构成, 这些数据保存在一个lua_State的结构中, 所有的
    lua应用都必须有一个lua_State. lua环境是用来发送和接收数据的地方, 利用lua
    栈来达到此目的。
> ### **1.1 lua控制台** ###
        程序请参见lua-console, 其功能类似于自己写一个lua的命令控制台, 需要注
    的是在lua5.1中luaopen_io被取消了; 我们可以使用luaL_openlibs来打开所有的
    lua标准库。。。
        void luaL_openlibs(lua_State* L);
