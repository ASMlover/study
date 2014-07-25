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
