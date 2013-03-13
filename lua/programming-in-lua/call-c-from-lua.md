# **从Lua调用C** #
***


## **1. C函数** ##
        实现在lua中调用C函数的一般步骤如下; 首先一个C函数如下:
        static int 
        l_sin(lua_State* L)
        {
          double d = lua_tonumber(L, 1);  /* 获取参数 */
          lua_pushnumber(L, sin(d));
          return 1;                       /* 结果输了 */
        }
        所有注册到Lua中的函数都具有相同的原型, 该函数类型的原型就定义在lua.h
    中的lua_CFunction:
        typedef int (*lua_CFunction)(lua_State* L);
        这样我们就可以将这个C函数注册到Lua环境中, 以便于在Lua中调用:
        lua_pushcfunction(L, l_sin);
        lua_setglobal(L, "l_sin");
        为了在实现C函数的时候对传递进来的参数进行检查; luaL_checknumber可以
    检查某个参数是否为一个数字, 如果不是就抛出一个错误信息, 如果是就返回该数
    字;
        static int 
        l_sin(lua_State* L)
        {
          double d = luaL_checknumber(L, 1);
          lua_pushnumber(L, sin(d));
          return 1;
        }



## **2. C模块** ##
    1) Lua调用C函数时, 不依赖于函数名, 包的位置或可见性规则, 而是依赖注册时
       传入的函数地址
    2) 当用C函数扩展lua时, 最好将代码设计为一个C模块; 即使现在只注册了一个函
       数, 但之后可能会需要更多的函数; 辅助库中提供了一个函数luaL_register, 
       可以接受一些C函数及其名称, 并将这些函数注册到一个与模块同名的table中
    3) DEMO:
       假设需要注册到Lua中的C函数如下:
        static int 
        l_sin(lua_State* L)
        {
          ...
        }
       然后声明一个数组, 其中包含模块中所有的函数和名称; 其类型为luaL_reg:
        static const struct luaL_reg mylib[] = {
          {"l_sin", l_sin}, 
          {NULL, NULL}, 
        };
       最后声明一个主函数, 其中用到luaL_register:
        int 
        luaopen_mylib(lua_State* L) 
        {
          luaL_register(L, "mylib", mylib);
          return 1;
        }
       luaL_register根据给定的名称("mylib")创建了一个table, 并用数组mylib中
       的信息填充这个table; luaL_register返回会将这个table留在栈中; 最后这个
       函数luaopen_mylib会返回1, 表示这个table返回给lua;

       写完这个模块后, 将这个模块编译为库, 并放到C路径中; 然后再LUA中就可以
       使用require来加载这个模块了:
        -- lua 
        require "mylib"
