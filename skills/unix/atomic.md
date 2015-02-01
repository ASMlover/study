# **Linux下的原子操作** #
***


## **1. 简单实现** ##
        我们知道在Windows下进行原子操作很简单, 直接使用Interlocked*系列的函
    数就可以很简单的实现;
        但是在Linux下应用层是不能直接使用的(也可能是我自己的知识局限???), 但
    是我们可以自己实现一套, 请参见`atomic.c`


## **2. GCC内置的原子操作** ##
> ### **2.1 背景** ###
        在Linux2.6.18之后, 删除了<asm/atomic.h>和<asm/bitops.h>, 但是GCC从版
    本4.1.2开始提供了内置的原子操作函数, 更适合用户态的程序使用; 而atomic.h
    在内核头文件之中, 不在GCC默认的搜索路径之下, 即是强制指定路径也会出现编
    译错误
> ### **2.2 GCC内置原子操作** ###
        GCC从4.1.2提供了__sync_*系列的built-in函数, 用于提供加减和逻辑运算的
    原子操作, 可以对1/2/4/8字节长度的数值类型或指针进行原子操作, 声明如下:
        type __sync_fetch_and_add(type* ptr, type value, ...)
        type __sync_fetch_and_sub(type* ptr, type value, ...)
        type __sync_fetch_and_or(type* ptr, type value, ...)
        type __sync_fetch_and_and(type* ptr, type value, ...)
        type __sync_fetch_and_xor(type* ptr, type value, ...)
        type __sync_fetch_and_nand(type* ptr, type value, ...)
            { tmp = *ptr; *ptr op = value; return tmp; }
            { tmp = *ptr; *ptr = ~tmp & value; return tmp; }  // nand
        type __sync_add_and_fetch(type* ptr, type value, ...)
        type __sync_sub_and_fetch(type* ptr, type value, ...)
        type __sync_or_and_fetch(type* ptr, type value, ...)
        type __sync_and_and_fetch(type* ptr, type value, ...)
        type __sync_xor_and_fetch(type* ptr, type value, ...)
        type __sync_nand_and_fetch(type* ptr, type value, ...)
            { *ptr op = value; return *ptr; }
            { *ptr = ~*ptr & value; return *ptr; }  // nand
        这两组函数的区别是, 第一组返回更新前的值, 第二组返回更新后的值;
> ### **2.3 应用** ###
        其应用也很简单, 和以前的atomic_*系列的函数类似, 但是对于那些使用了
    atomic.h的老代码, 可以使用宏定义的方式, 一直到高内核版本的Linux系统上:
        #define atomic_inc(x)     __sync_add_and_fetch((x), 1)
        #define atomic_dec(x)     __sync_sub_and_fetch((x), 1)
        #define atomic_add(x, y)  __sync_add_and_fetch((x), (y))
        #define atomic_sub(x, y)  __sync_sub_and_fetch((x), (y))


## **3. Windows&Linux兼容的Atomic**
    请见cross_platform_atomic.h
