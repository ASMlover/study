# **newLisp共享库** #
***



## **newLisp和C语言互相调用** ##
    1) newlisp中调用C语言中的函数, 可以使用(import lib-name func-name)来直接
       使用C共享库中的函数
    2) 要在C语言中使用newlisp, 可以用newlisp.dll(Windows)/newlisp.so(Linux)/
       newlisp.dylib(Mac OS X);
       其中有一个函数const char* newlispEvalStr(const char*);
    3) 具体例子请参见./src/newlisp01/
    4) Linux版本的具体例子请参见./src/newLisp02/
