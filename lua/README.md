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
