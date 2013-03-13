# **编写C函数的技术** #
***


## **1. 数组操作** ##
    1) Lua中, 数组只是table的一个别名, 是指以一种特殊的方法来使用table
    2) API为数组操作提供了专门的函数; 
       首先, 出于性能考虑, 通常会在算法中用循环来访问函数;
       其次, 为了方便;
    3) API为数组操作提供的函数如下:
       void lua_rawgeti(lua_State* L, int index, int key);
       void lua_rawseti(lua_State* L, int index, int key);
       这两个函数都是原始操作, 比涉及元表的table访问更快
    4) 例子, 其作用是时下一个交换map, 对数组中每一个元素应用一个指定的函数后
       将结果写到原有的数组元素上:
        int 
        l_map(lua_State* L)
        { 
          int i, n;

          luaL_checktype(L, 1, LUA_TTABLE); /* 第一个参数必须是table */
          luaL_checktype(L, 2, LUA_TFUNCTION);
          n = lua_objlen(L, 1);   /* 获取table的大小 */

          for (i = i; i <= n; ++i) {
            lua_pushvalue(L, 2);  /* 压入f */
            lua_rawgeti(L, 1, i); /* 压入t[i] */
            lua_call(L, 1, 1);    /* 调用f(t[i]) */
            lua_rawseti(L, 1, i); /* t[i] = 结果 */
          }

          return 0;
        }



## **2. 字符串操作** ##
    1) 当一个C函数从Lua收到一个字符串参数时, 必须: 不要在访问字符串时从栈中
       弹出, 不要修改字符串
    2) 一个C函数需要创建一个字符串返回给Lua时, C代码还必须处理字符串缓冲的分
       配和释放, 缓冲溢出等问题
    3) lua_pushlstring支持提取子串, 接受一个额外的字符串长度参数
    4) lua_concat用于连接字符串, 类似于lua中的".."; 可以同时连接多个字符串, 
       调用lua_concat(L, n)连接并弹出栈顶的n个值, 然后压入结果;
       这个函数会将数字转换为字符串, 并在需要时调用元方法
    5) lua_pushfstring类似于sprintf, 会根据一个格式字符串和一些额外的参数来
       创建一个新字符串, 且无需提供这个新字符串的缓冲
       const char* lua_pushfstring(lua_State* L, const char* fmt, ...);
    6) 使用缓冲机制, 首先需要声明一个luaL_Buffer的变量
           luaL_buffinit用于初始化, 初始化后, 这个变量会保留一份状态L的副本,
       由此在调用其他操作缓冲的函数就无需传递状态参数了
           void luaL_buffinit(lua_State* L, luaL_Buffer* B);

           luaL_addchar会将一个字符放入缓冲
           void luaL_addchar(luaL_Buffer* B, char c);

           luaL_addlstring将具有显示长度的字符串放入到缓冲
           void luaL_addlstring(luaL_Buffer* B, const char* s, size_t l);

           luaL_addstring将以"0"结果的字符串放入到缓冲
           void luaL_addstring(luaL_Buffer* B, const char* s);

           luaL_pushresult会更新缓冲, 并就将最终的字符串留在栈顶
           void luaL_pushresult(luaL_Buffer* B);

           luaL_addvalue将栈顶的值加入到缓冲, 如果栈顶不是字符串或数字, 那么
       调用这个函数就会是一个错误
           luaL_addvalue(luaL_Buffer* B);



## **3. 在C函数中保存状态** ##
    1) Lua函数, 有3种地方可以存放非局部的数据: 全局变量, 函数环境和非局部的
       变量(closure中)
    2) C API也提供了3种地方来保存这类数据: 注册表, 环境和upvalue
    3) 注册表示一个全局table, 只能被C代码访问; 用来保存需要在几个模块中共享
       的数据
    4) 环境用来保存一个模块的私有数据; 每个C函数都有自己的环境table; 一个模
       块内的所有函数共享同一个环境table, 由此他们可以共享数据
    5) C函数也可以拥有upvalue, 其是一种与特定函数想关联的Lua值
> ### **3.1 注册表** ###
    1) 总是位于一个伪索引上, 索引值由LUA_REGISTRYINDEX定义
    2) 获取注册表中key为"key"的值, 可以
       lua_getfield(L, LUA_REGISTRYINDEX, "key");
    3) 注册表是一个普通table, 可以使用任何Lua值(nil除外)来索引它
    4) 不应该使用数字类型的key, 这种key是被"引用系统"保留的
> ### **3.2 C函数的环境** ###
    1) 环境table的伪索引是LUA_ENVIRONINDEX
    2) 先为模块创建一个新的table, 然后模块中的所有函数都共享这个table
> ### **3.3 upvalue** ###
    1) upvalue实现了一种类似C语言中静态变量的机制, 其只在一个特定函数中可见
    2) 在lua中创建一个函数时, 都可以将任意数量的upvalue与这个函数相关联
    3) 例子请见原书...
