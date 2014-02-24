# **README for fork**
***



## **Summary**
 * 记录关于fork的小技巧或疑问?



## **原题**
      #include <sys/types.h>
      #include <unistd.h>
      #include <stdio.h>

      int main(void) {
        int i;
        for (i = 0; i < 2; i++) {
          fork();
          printf("-");
        }

        return 0;
      }
      问总共输出几个'-'?  => 8个
      如果将printf("-")修改为printf("-\n")则是6个, 因为有行缓冲;




## **分析**
    fork系统调用的特性:
    1. 是unix下以自身进程创建子进程的系统调用, 一次调用两次返回; 返回0是子进
       程, 返回结果>0则是父进程(返回值是子进程pid)
    2. fork调用处, 整个父进程空间会原模原样的复制到子进程空间, 包括指令, 变
       量值, 程序调用栈, 环境变量和缓冲区等等...


## **参考**
 * [fork面试](http://coolshell.cn/articles/7965.html)
