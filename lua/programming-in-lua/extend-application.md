# **扩展应用程序** #
***

## **1. 基础** ##
        加入lua中的配置如下:
        -- lua 
        width = 200
        height = 300
        则需要在C语言中来读取lua中的配置, 读取函数可以如下所示:
        // C 
        void 
        load(lua_State* L, const char* fname, int* w, int* h) 
        { 
          if (luaL_loadfile(L, fname) || lua_pcall(L, 0, 0, 0))
            error(L, "cannot run config. %s", lua_tostring(L, -1));
          
          lua_getglobal(L, "width");
          lua_getglobal(L, "height");
          if (!lua_isnumber(L, -2))
            error(L, "'width' should be a number");
          if (!lua_isnumber(L, -1))
            error(L, "'height' should be a number");
          
          *w = lua_tointeger(L, -2);
          *h = lua_tointeger(L, -1);
        }

        在lua中使用配置的理由是, 其更易于将新的配置机制添加到程序中, 可以使
    程序变得更加灵活



## **2. table操作** ##
    1) 当配置一个窗口的RGB值的时候, 如果有多个, 全设置成全局变量的时候:
        太冗长;
        无法预定义常用颜色;
        这个时候就可以使用table 
        -- lua 
        background = {r=0.3, g=0.1, b=0}
        这样在C语言中获取table值的方法如下:
        /* C */
        lua_getglobal(L, "background");
        if (!lua_istable(L, -1))
          error(L, "'background' is not a table");
        r = getfield(L, "r");
        g = getfield(L, "g");
        b = getfield(L, "b");

        getfield的具体实现如下:
        #define MAX_COLOR   (255)

        int 
        getfield(lua_State* L, const char* key)
        {
          int r;
          lua_pushstring(L, key);
          lua_gettable(L, -2);    /* 获取background[key] */
          if (!lua_isnumber(L, -1))
            error(L, "invalid component in background color");
          r = (int)lua_tonumber(L, -1) * MAX_COLOR;
          lua_pop(L, 1)           /* 删除数字 */
          return r;
        }
        getfield假设table在栈顶, 当lua_pushstring压入key之后, table就在-2的
    位置上, 在返回前, 需要弹出从栈中检索到的值, 并使栈恢复原来的样子;
        由于经常需要使用字符串来索引table, lua5.1中提供了一个lua_gettable的
    的特化版本lua_getfield, 所以可以将
        lua_pushstring(L, key);
        lua_gettable(L, -2);
        替换为:
        lua_getfield(L, -1, key);
    2) 当然可以在C语言中定义一个table, 然后写到lua中
        struct ColorTable {
          char* name;
          unsigned char r, g, b;
        } colortable[] = {
          {"WHITE", MAX_COLOR, MAX_COLOR, MAX_COLOR}, 
          {NULL, 0, 0, 0}
        };
        其最终到lua中应该如:
        WHITE = {r=1, g=1, b=1}
        其具体实现如下:
        void 
        setfield(lua_State* L, const char* index, int v)
        {
          lua_pushstring(L, index);
          lua_pushnumber(L, (double)v / MAX_COLOR);
          lua_settable(L, -3);
        }
        该函数假设在调用前, table就已经是栈顶; 所以可以修改如下:
        void 
        setfield(lua_State* L, const char* index, int v)
        {
          lua_pushnumber(L, (double)v / MAX_COLOR);
          lua_setfield(L, -2, index);
        }
        那么设置颜色的函数就可以出现了
        void
        setcolor(lua_State* L, struct ColorTable* ct)
        {
          lua_newtable(L);  /* 创建一个table */
          setfield(L, "r", ct->r);
          setfield(L, "g", ct->g);
          setfield(L, "b", ct->b);
          lua_setglobal(L, ct->name);
        }



## **3. 调用Lua函数** ##
        假设lua中函数如下:
        -- lua
        function f(x, y)
          return (x ^ 2 * math.sin(y)) / (1 - x)
        end 
        那么在C语言中调用如下:
        double 
        f(double x, double y)
        {
          double z;

          /* 压入函数和参数 */
          lua_getglobal(L, "f");
          lua_pushnumber(L, x);
          lua_pushnumber(L, y);

          /* 完成调用 (2个参数1个结果) */
          if (lua_pcall(L, 2, 1, 0) != 0)
            error(L, "error : %s", lua_tostring(L, -1));

          if (!lua_isnumber(L, -1))
            error(L, "'f' must return a number");
          z = lua_tonumber(L, -1);
          lua_pop(L, 1);

          return z;
        }



## **4. 一个通用的调用函数** ##
        具体实现减书籍, 书籍只是提供了作者的一个实现, 我们可以使用另外的实现
    毕竟lua API是公开的, 我们可以根据自己的喜好来设置自己的实现...
