# **扩展**
***



## **1. C API**
> ### **1.1 基础**
    1) 解释器程序:
        int main(int argc, char* argv[]) {
          char buff[256];
          int error;
          lua_State* L = luaL_newstate();
          luaL_openlibs(L);

          while (fgets(buff, sizeof(buff), stdin) != NULL) {
            error = luaL_loadbuffer(L, buff, strlen(buff), "line") || 
                lua_pcall(L, 0, 0, 0);

            if (error) {
              fprintf(stderr, "%s", lua_tostring(L, -1));
              lua_pop(L, 1);
            }
          }

          lua_close(L);
          return 0;
        }
    2) lua.h定义了Lua提供的基础函数(lua环境, 调用lua函数, 读写lua环境中的全
       局变量, 注册lua调用的新函数)
    3) lauxlib.h定义了辅助库提供的函数;

> ### **1.2 栈**
    1) 通过栈来交换数据
        * 获取一个值, 只需调用一个Lua API, lua会将指定值压入栈中;
        * 传值, 需要先将值压入栈, 然后调用Lua API, Lua会将获取该值并将其从栈
          中弹出;
        * 严格的LIFO操作, 调用Lua时只会改变栈的顶部;
    2) 压入元素
        * 常量nil => lua_pushnil
        * 双精度浮点数 => lua_pushnumber
        * 整数 => lua_pushinteger
        * 布尔 => lua_pushboolean
        * 字符串(任意字符串, char*及长度) => lua_pushlstring
        * 零结尾的字符串 => lua_pushstring
    3) 向栈中压入元素的时候, 应该确保栈中有足够的空间; lua启动时栈中至少有20
       个空闲的槽; 
       lua_checkstack(lua_State* L, int sz)可以检查栈中是否有足够空间;
    4) 查询元素:
        * 第一个压入元素为1, 第二个为2, 依次类推到栈顶; 正数访问从栈底开始;
        * -1为栈顶, -2为栈顶下的元素, 依次类推到栈底, 负数访问从栈顶开始;
        * lua_is*类函数检查元素是否为特定类型;
        * lua_type返回元素类型:
            - LUA_TNIL
            - LUA_TBOOLEAN
            - LUA_TNUMBER
            - LUA_TSTRING
            - LUA_TTABLE
            - LUA_TTHREAD
            - LUA_TUSERDATA
            - LUA_TFUNCTION
        * lua_to*类函数从栈中获取一个值;
    5) 其他操作
        * lua_gettop => 返回栈中元素个数, 也就是栈顶元素的索引;
        * lua_settop => 将栈顶设置到一个指定位置, 修改栈中元素的数量;
          lua_settop(L, 0)用于清空栈;
        * lua_pushvalue => 将指定索引上值的副本压入栈;
        * lua_remove => 删除指定索引上的元素, 并将该位置之上的所有元素下移以
          填补空缺;
        * lua_insert => 上移指定位置上的元素以开辟一个槽的空间, 然后将栈顶元
          移动到该位置;
        * lua_replace => 弹出栈顶的值, 将该值设置到指定索引上,不移动任何东西

> ### **1.3 C API中的错误处理**
    1) 遇到错误, 几乎所有API会抛出错误, 而不是返回错误;
    2) 发生错误时, lua会调用紧急函数, 当这个函数返回后, lua就结束程序; 我们
       可以通过lua_atpanic来设置自己的紧急函数
    3) 应对错误:
        * 设置自己的紧急函数
        * 通过lua_pcall来运行lua代码, 让代码在保护模式运行;
    4) 当一个C函数监测到一个错误应该调用lua_error;
        lua_error会清理lua中所需要清理的东西, 然后跳转回发起执行的那个
        lua_pcall, 并附上一条错误信息;



## **2. 扩展应用程序**
> ### **2.1 基础**
    1) 获取lua中的变量:
          -- defined in lua fil 
          width = 200
          height = 300
          // in C++
          void load(lua_State* L, const char* fname, int* w, int* h) {
            if (luaL_loadfile(L, fname) || lua_pcall(L, 0, 0, 0))
              error(...);
            lua_getglobal(L, "width");
            lua_getglobal(L, "height");
            if (!lua_isnumber(L, -2))
              error(...);
            if (!lua_isnumber(L, -1))
              error(...);
            *w = lua_tointeger(L, -2);
            *h = lua_tointeger(L, -1);
          }

> ### **2.2 table操作**
    1) 获取table变量中的成员:
          -- Lua 
          background = {r=0.3, g=0.1, b=0.2}
          // C++
          lua_getglobal(L, "background")；
          if (!lua_istable(L, -1))
            error(...);
          red = getfield(L, "r");
          green = getfield(L, "g");
          blue = getfield(L, "b");
          // getfield 实现如下
          int getfield(lua_State* L, const char* key) {
            int result;
            lua_pushstring(L, key);
            lua_gettable(L, -2);  // get background[key];
            if (!lua_isnumber(L, -1))
              error(...);
            result = (int)lua_tonumber(L, -1);
            lua_pop(L, 1);
            return result;
          }
    2) 在lua5.1中可以更简单的实现:
          lua_getfield(L, -1, key);
          可以替换掉
          lua_pushstring(L, key);
          lua_gettable(L, -2);
    3) 写table
          void SetTable(lua_State* L, const char* index, int value) {
            lua_pushstring(L, index);
            lua_pushnumber(L, value);
            lua_settable(L, -3);
          }
          void SetTable(lua_State* L, const char* index, int value) {
            lua_pushnumber(L, value);
            lua_setfield(L, -2, index);
          }
