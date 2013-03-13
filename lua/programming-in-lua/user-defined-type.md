# **用户自定义类型** #
*** 


## **1. userdata ** ##
    1) userdata提供了一块原始的内存区域, 可以用来存储任何东西
    2) lua中userdata没有任何预定义的操作
    3) lua_newuserdata会根据指定的大小分配一块内存, 并将对于的userdata压入栈
       中, 最后返回这个内存块的地址:
       void* lua_newuserdata(lua_State* L, size_t size);


## **2. 元表** ##
    1) 辨别不同类型的userdata的方法是为每种类型创建一个唯一的元表;
       每当创建一个userdata后就用相应的元表来标记它;
    2) luaL_newmetatable会创建一个新的table用作元素, 并将其压入栈顶, 然后将
       这个table与注册表中的指定名称关联起来


## **3. 面向对象的访问** ##
        在C语言中实现面向对象的模块, 以便在lua中调用:
        static const struct luaL_Reg arraylib_f[] = {
          {"new", newarray}, 
          {NULL, NULL}
        };
        static const struct luaL_Reg arraylib_m[] = {
          {"set", setarray}, 
          {"get", getarray}, 
          {"size", getsize}, 
          {NULL, NULL}
        };
        int 
        luaopen_array(lua_State* L)
        {
          lua_newmetatable(L, "LuaBook.array");
          lua_pushvalue(L, -1);
          lua_setfield(L, -2, "__index");

          luaL_register(L, NULL, arraylib_m);
          luaL_register(L, "array", arraylib_f);

          return 1;
        }



## **4. 数组访问** ##
    1) 另一种面向对象写法是使用常规的数组访问写法
    2) setarray和getarray可以使用类a[i]的方式来访问
        static const struct luaL_Reg arraylib_f[] = {
          {"new", newarray}, 
          {NULL, NULL}
        };
        static const struct luaL_Reg arraylib_m[] = {
          {"__newindex", setarray}, 
          {"__index", getarray}, 
          {"__len", getsize}, 
          {"__tostring", array2string}, 
          {NULL, NULL}
        };
        int 
        luaopen_array(lua_State* L)
        {
          luaL_newmetatable(L, "LuaBook.array");
          luaL_register(L, NULL, arraylib_m);
          luaL_register(L, "array", arraylib_f);
          return 1;
        }


## **5. 轻量级userdata** ##
    1) 是一种表示C指针的值
    2) 将轻量级userdata放到栈中
       void lua_pushlightuserdata(lua_State* L, void* p);
    3) 轻量级userdata不是缓冲, 只是一个指针, 没有元表, 不需要受垃圾收集器的
       管理
