# **内存管理** #
***


## **1. 分配函数** ##
    1) lua核心不会对内存分配方式作任何假设, 不会调用malloc或realloc来分配内
       存, 而是通过一个分配函数来完成所有的内存分配和释放; 当用户创建一个lua
       状态的时候, 必须提供这个函数
    2) 要获取对Lua内存分配的完全控制, 只需要使用lua_newstate来创建状态就可以
       了
       lua_State* lua_newstate(lua_Alloc f, void* ud);
       typedef void* （*lua_Alloc)(void* ud, void* ptr, 
                                   size_t osize, size_t nsize);
    3) lua_getallocf可以获取一个Lua状态的内存分配函数
       lua_Alloc lua_getallocf(lua_State* L, void** ud);
       如果ud不为NULL, 函数会将*ud设为这个分配函数的用户数据
    4) lua_setallocf可以修改一个lua状态的内存分配函数
       void lua_setallocf(lua_State* L, lua_Alloc f, void* ud);


## **2. 垃圾收集器** ##
    1) lua为了完成一个完成的垃圾收集周期而暂停与主程序的交互, 每个周期分为:
       标记(mark), 整理(cleaning), 清扫(sweep), 收尾(finalization)
> ### **2.1 原子操作** ###
    1) 主要的原子操作是table的遍历和整理阶段
    2) 原子的table遍历表示收集器在遍历一个table时是不会停止的
    3) 当一个程序具有一个极大的table时, 就需要将这个table分成一些较小的部分
    4) table中每个条目的大小不影响table遍历, 而是条目的数量
> ### **2.2 垃圾收集器的API** ###
    1) C语言中
       int lua_gc(lua_State* L, int what, int data);
       lua中
       collectgarbage(what[, data])

       what指定了要做的事情, 有:
       LUA_GCSTOP("stop"), 停止收集器, 知道在此以"restart", "collect"或者以
       "step"来调用collectgarbage为止;
       LUA_GCRESTART("restart"), 重启收集器;
       LUA_GCCOLLECT("collect"), 执行一轮完整的垃圾收集周期, 收集并释放所有
       不可能到达的对象(这个是collectgarbage的默认选项);
       LUA_GCSTEP("step"), 执行一些垃圾收集的工作;
       LUA_GCCOUNT("count"), 返回lua当前使用的内存数量, 以千字节为单位;
       LUA_GCCOUNTB, 返回lua当前使用的内存数量的千字节余数;
       LUA_GCSETPAUSE("setpause"), 设置收集器的pause参数, 其值由data参数指定
       表示一个百分比;
       LUA_GCSETSTEPMUL("setstepmul"), 设置收集器的stepmul参数, 其值也是由
       data参数指定, 表示一个百分比;
