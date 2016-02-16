# **Flex规范参考**
***

## **1. Flex结构规范**
Flex程序由3部分组成：定义部分、规则部分和用户子例程。
```c
... 定义部分 ...
%%
... 规则部分 ...
%%
... 用户子例程 ...
```
  * 定义部分包含选项、文字块、定义、开始条件和转换。
  * 规则部分包括模式行和C代码。以空白符开始或处于%{和%}之间的内容被认为是C代码，会被原封不动的拷贝到yylex()中。在规则部分开头出现的C代码也会出现在yylex()的开头，可以包含词法分析器中需要使用的变量的声明，以及每次调用yylex()所需要运行的代码。
  * 用户子例程的内容将被Flex原样拷贝到C文件，通常包含规则中需要调用的例程。
  * BEGIN宏切换起始状态，词法分析器从状态0开始，该状态称为INITIAL，其他所有状态必须在定义部分通过%s或%x行来命名。

## **2. 输入管理**
可以将任何打开的标准输入文件赋予yyin，这样词法分析器就会从该文件读取。可以通过yyrestart(file)来使得词法分析器读取任意标准输入文件。
```c
// 可从一个输入缓冲区中读取输入, YY_BUFFER_STATE指向Flex输入缓冲区的指针
YY_BUFFER_STATE bp;
FILE* f = fopen(..., "r");
bp = yy_create_buffer(f, YY_BUF_SIZE); // 从f创建新缓冲区, 必须是YY_BUF_SIZE
yy_switch_to_buffer(bp);  // 使用我们刚创建的缓冲区
...
yy_flush_buffer(bp);      // 放弃缓冲区中的内容
...
yy_delete_buffer(bp);     // 释放缓冲区
```
通常Flex从文件读取输入，有时你希望从其他来源进行读取，比如内存字符串：
```c
bp = yy_scan_bytes(bytes, len); // 分析字节流拷贝
bp = yy_scan_string("string");  // 分析以空字符结尾的字符串拷贝
bp = yy_scan_buffer(char* base, yy_size_t size); // 分析长度为(size-2)的字节流
```
Flex提供了一对函数来管理输入缓冲区堆栈：
```c
void yypush_buffer_state(bp); // 切换到bp，把旧的缓冲区压入堆栈
void yypop_buffer_state();    // 删除当前缓冲区，继续使用上一个缓冲区
```
Flex通过`YY_INPUT(buf, result, max_size)`来读取输入到一个缓冲区，一旦词法分析器发现需要更多的输入而缓冲区为空的时候就会调用`YY_INPUT`，当缓冲区第一次被简历起来的时候，词法分析器将调用isatty()来确定输入源是否来自终端，如果是将每次读取一个字符而不是一大段字符。
