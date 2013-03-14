# **管理资源** #
***


## **1. 目录迭代器** ##
    1) 实现后在Lua中的使用模式如下:
        for fname in dir('.') do
          print(fname)
        end
    2) 目录迭代器实现如下(Linux下):
        static int 
        l_dir(lua_State* L)
        {
          const char* path = luaL_checkstring(L, 1);
          DIR** d = (DIR**)lua_newuserdata(L, sizeof(DIR*));

          luaL_getmetatable(L, "LuaBook.dir");
          lua_setmetatable(L, -2);

          *d = opendir(path);
          if (NULL == *d)
            luaL_error(L, "connot open %s: %s", path, strerror(errno));
          
          lua_pushcclosure(L, dir_iter, 1);

          return 1;
        }

        static int 
        dir_iter(lua_State* L)
        {
          DIR* d = *(DIR**)lua_touserdata(L, lua_upvalueindex(1));
          struct dirent* entry;

          if (NULL != (entry = readdir(d))) {
            lua_pushstring(L, entry->d_name);
            return 1;
          }
          
          return 0;
        }

        static int 
        dir_gc(lua_State* L)
        {
          DIR* d = *(DIR**)lua_touserdata(L, 1);
          if (NULL != d)
            closedir(d);
          
          return 0;
        }

        int 
        luaopen_dir(lua_State* L)
        {
          luaL_newmetatable(L, "LuaBook.dir");

          lua_pushstring(L, "__gc");
          lua_pushcfunction(L, dir_gc);
          lua_settable(L, -3);

          lua_pushcfunction(L, l_dir);
          lua_setglobal(L, "dir");

          return 0;
        }


## **2. XML分析器** ##
    具体例子请见书...
