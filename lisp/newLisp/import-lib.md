# **Importing Libraries** #
***


## **import** ##
    1) 语法
        (import str-lib-name str-function-name ["cdecl"])
        (import str-lib-name str-function-name str-return-type 
          [str-param-type ... ])
        (import str-lib-name)
    2) 使用crt库
        (define LIBC (loopup ostype '(
          ("Win32" "mscrt.dll")
          ("Linux" "libc.so.6")
          ("OSX" "libc.dylib"))))
        根据平台来定义LIBC到底指向那个动态库
        然后就可以使用import来引用crt中的函数了
        具体例子请参见./src/newlisp00/
