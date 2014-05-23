# **README for lua** #
***


## **说明** ##
    这里主要记录了Lua的相关知识, 以及使用Lua进行游戏开发的一些知识点...


## **编译lua**
    编译成共享库
    1. 确保不包含lua.o luac.o print.o
    2. CFLAGS选项中加入 -fPIC
        CFLAGS = -Wall -O2 -fPIC
    3. $(LUA_SO): $(SO_OBJS)
        $(CC) -o $@ -shared $? -ldl -lm



## **Lua的栈索引**
            栈顶
          5  |  -1
          4  |  -2
          3  |  -3
          2  |  -4
          1  |  -5
            栈底
    1. 正数索引, 不需要知道栈的大小, 就知道栈底在哪, 永远为1
    2. 负数索引, 不需要知道栈的大小, 就知道栈顶在哪, 永远为-1
