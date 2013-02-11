# **错误和异常** #
***


## **1. 错误类型和基本调试方法** ##
> ### **1.1 PHP错误报告等级** ###
    1) E_ERROR: 致命运行时错误
    2) E_WARNING: 运行时警告
    3) E_PARSE: 从语法中解析错误
    4) E_NOTICE: 运行时注意消息
    5) E_CORE_ERROR: 不包括PHP核心造成的错误
    6) E_CORE_WARNING: 不包括PHP核心警告
    7) E_COMPILE_ERROR: 编译时错误
    8) E_COMPILE_WARNING: 编译时警告
    9) E_USER_ERROR: 用户导致的错误
    10) E_USER_WARNING: 用户导致的警告
    11) E_USER_NOTICE: 用户导致的注意消息
    12) E_ALL: 所有的错误、警告和注意
    13) E_STRICT: 关于PHP版本移植的兼容性和互操作性建议
> ### **1.2 设置错误报告级别** ###
    1) 在php.ini中配置error_reporting
    2) 使用error_reporting函数来设置错误报告级别



## **2. 错误日志** ##
> ### **2.1 使用指定的文件记录错误报告日志** ###
    1) php.ini中指定错误日志文件, 设置的时候需要使用错误日志文件的绝对路径, 
       error_log = /usr/local/error.log, 并设置Web服务器进程用户有写权限
       bool error_log(string msg[, int msg_type[, 
                      string destination[, string extra_headers]]]);
       该函数会将错误信息发送到Web服务器的错误日志文件中
    2) 错误信息记录到操作系统的日志里
        Linux上错误信息发送到syslog, Windows错误信息发送到事件日志中;
        php.ini中配置为:
          error_reporting = E_ALL
          display_errors = Off
          log_errors = On
          log_errors_max_len = 1024
          error_log = syslog
        define_syslog_variables => 
          在使用openlog, syslog和closelog之前需先调用该函数
        openlog => 
          打开一个和当前系统中日志器的连接, 为向系统插入日志消息做好准备
        syslog => 
          向系统日志发送一个定制消息
        closelog => 
          向系统日志中发送完成定制消息以后调用, 关闭由openlog打开的日志连接



## **3. 异常处理** ##
> ### **3.1 异常处理实现** ###
        try {
          ...
        } catch(ex1) {
          ...
        }
> ### **3.2 扩展PHP内置的异常处理类** ###
        在try代码中, 需要使用throw语句抛出一个异常对象, 才能跳转到catch代码
    块中执行, 并在catch代码块中捕获并使用这个异常类的对象。
> ### **3.3 捕获多个异常** ###
        在try代码之后, 必须至少给出一个catch代码块, 也可以将多个catch代码块
    与一个try代码块进行关联。
