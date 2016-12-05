# **InitOnce**
***

## **在Windows上实现类似`pthread_once`一样的功能**
    1、在Vista以上的机器使用InitOnceExecuteOnce函数来实现
    2、在XP(虽然现在也不支持XP了)上可以使用Interlocked*的函数来实现
``` c
    typedef int32_t once_t;
    void init_once(once_t* once, void (*initializer)(void)) {
        if (0 == InterlockedCompareExchange(once, 1, 0))
            initializer();
    }
```
