# **Unix下的共享库** #
*** 



## **1. Linux下创建共享库** ##
    1) 使用gcc命令如下:
        $ gcc -o $*.o -c -fPIC $^ 
        $ gcc -o $@ $^ -shared
    2) 具体例子请参见./shared-demo/ 



## **2. 共享库的使用** ##
    1) gcc -o $@ $^ -lxxx
    2) 具体例子请参见./shared-demo/
