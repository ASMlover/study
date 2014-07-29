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

> ### **2.3 调用lua函数**
        -- Lua function
        function f(x, y)
        // C++
        double x, y;
        lua_getglobal(L, "f");
        lua_pushnumber(L, x);
        lua_pushnumber(L, y);
        if (lua_pcall(L, 2, 1, 0) != 0)
          error(...);
        if (!lua_isnumber(L, -1))
          error(...);
        r = lua_tonumber(L, -1);
        lua_pop(L, 1);




## **3. 从Lua调用C**
> ### **3.1 C函数**
    1) 所有注册到Lua中的C函数都必须是如下类型:
        typedef int (*lua_CFunction)(lua_State*);
        返回一个整数表示压入到栈中的返回值的数量;
    2) 在Lua使用C函数之前, 必须注册这个函数, 可以使用lua_pushcfunction来注册
        lua_pushcfunction(L, l_sin);
        lua_setglobal(L, "my_sin");

> ### **3.2 C模块**
    1) Lua调用C函数时, 并不依赖函数名, 包的位置或可见性规则, 而只依赖于注册
       时传入的函数地址; C模块只有一个公共函数, 用于创建C模块;
    2) 步骤:
        * 定义模块的函数;
        * 声明一个luaL_Reg的数组, 其中包含模块中所有的函数和名称:
          static const struct luaL_Reg mylib[] = {
            {"dir", l_dir}, 
            {"sin", l_sin}, 
            {NULL, NULL},
          }
          数组最后一个元素总是{NULL, NULL}用以标识结尾;
        * 声明模块主函数:
          int luaopen_mylib(lua_State*) {
            luaL_register(L, "mylib", mylib);
            return 1;
          }
          在luaL_register返回的时候, 会将这模块table留在栈中;
        * 在Lua中可以使用require "mylib"加载这个模块;



## **4. 编写C函数的技术**
> ### **4.1 数组操作**
    1) API
        void lua_rawgeti(lua_State* L, int index, int key);
        void lua_rawseti(lua_State* L, int index, int key);
        index表示table在栈中的位置, key表示元素在table中的位置;

> ### **4.2 字符串操作**
    1) 一个C函数从Lua中收到一个字符串参数时:
        * 不要在访问字符串时从栈中弹出它;
        * 不要修改字符串;
    2) 有用的函数:
        * lua_pushfstring(lua_State* L, const char* fmt, ...);
          类似于sprintf, 会根据一个格式字符串和一些额外的参数来创建一个新字
          符串且无需提供这个字符串的缓冲;
        * 只接受%%, %s, %d, %f, %c; 不接受任何例如宽度或精度选项;
    3) 使用缓存机制的第一步是声明一个luaL_Buffer变量, 并用luaL_buffinit来初
       始化
        * luaL_addchar -> 将一个字符放入缓存
        * luaL_addlstring -> 将具有显示长度的字符串放入缓存
        * luaL_addstring -> 将0结尾的字符串放入缓存
        * luaL_pushresult -> 更新缓存, 并将最终的字符串留在栈顶
    4) luaL_addvalue用于将栈顶的值加入缓冲, 如果栈顶的值不是字符串或数字的话
       那么调用这个函数会引发一个错误;

> ### **4.3 在C函数中保存状态**
        在C中编写Lua库的时候最好不要使用全局变量或静态变量:
          * 无法在一个C变量中保存普通的Lua对象;
          * 若一个库使用全局变量或静态变量, 就无法用于多个Lua状态了;
    1) 注册表(Registry)
        * 位于伪索引上, 由LUA_REGISTRYINDEX定义 
        * lua_getfield(L, LUA_REGISTRYINDEX, key)获取key对应的值;
        * 不应该使用数字类型的Key, 这种key被引用系统所保留;
        * luaL_ref(L, LUA_REGISTRYINDEX)会从栈中弹出一个值, 用一个新分配的整
          数key来将这个值保存到注册表中, 最后返回这个Key;
        * lua_rawgeti(L, LUA_REGISTRYINDEX, r)将与引用r关联的值压入栈中;
        * luaL_unref(L, LUA_REGISTRYINDEX, r)释放该值和引用;



## **5. 用户自定义类型**
> ### **5.1 userdata**
    1) lua_newuserdata会根据指定大小分配一块内存, 并将对应的userdata压入栈中
       最后返回这个内存块的地址;

> ### **5.2 元表**
    1) 辨别不同类型的userdata的方法是为每种类型创建一个唯一的元表;
    2) Lua中习惯将所有新的C类型注册到注册表中, 以一个类型名作为key, 元表作为
       value;
    3) 辅助库函数
        * luaL_newmetatable -> 创建一个新的table做元表, 将其压入栈顶, 然后将
          这个table与注册表中的指定名称关联起来
        * luaL_getmetatable -> 可以在注册表中检索与tname管理的元表;
        * luaL_checkudata -> 可以检查栈中指定位置上是否为一个uerdata, 并是否
          具有与给定名称相匹配的元表;

> ### **5.3 面向对象的访问**
        static const strutc luaL_Reg arraylib_f[] = {
          {"New", NewArray}, 
          {NULL, NULL},
        };
        static const struct luaL_Reg arraylib_m[] = {
          {"Set", SetArray}, 
          {"Get", GetArray},
          {"Size", GetSize}, 
          {NULL, NULL},
        };
        int luaopen_array(lua_State* L) {
          luaL_newmetatable(L, "LuaBook.Array");
          lua_pushvalue(L, -1); // 复制元表
          lua_setfield(L, -2, "__index");
          luaL_register(L, NULL, arraylib_m);
          luaL_register(L, "Array", arraylib_f);
          return 1;
        }

> ### **5.4 数组访问**
        static const struct luaL_Reg arraylib_f[] = {
          {"New", NewArray}, 
          {NULL, NULL},
        };
        static const struct luaL_Reg arraylib_m[] = {
          {"__newindex", SetArray}, 
          {"__index", GetArray}, 
          {"__len", GetSize}, 
          {"__tostring", Array2String}, 
          {NULL, NULL}, 
        };
        int luaopen_array(lua_State* L) {
          luaL_newmetatable(L, "LuaBook.Array");
          luaL_register(L, NULL, arraylib_m);
          luaL_register(L, "Array", arraylib_f);
          return 1;
        }
