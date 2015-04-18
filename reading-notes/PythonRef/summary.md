# **Python Essential Reference**
***

## **1. Python语言**
> ### **1.1 简介**
    1) 变量和表达式
    2) 条件
    3) 输入和输出
    4) 数据结构：
        * 字符串
        * 列表
        * 元组
        * 集合
        * 字典
    5) 迭代和循环
    6) 函数
    7) 生成器
    8) 协程
    9) 对象和类
    10) 异常处理
    11) 模块
> ### **1.2 词汇和语法**
    1) 行结构和缩进
    2) 标示符和保留字
    3) 数字字面量
    4) 字符串字面量
    5) 容器
    6) 运算符、分隔符以及特殊符号
    7) 文档字符串
    8) 装饰器
    9) 源代码编码(utf-8等)
> ### **1.3 类型与对象**
    1) 对象的身份和类型
    2) 引用计数与垃圾收集
    3) 引用与复制(reference and value copy)
    4) 第一类对象
    5) 内置数据类型(None, 数字, List, Dict, Set)
    6) 内置程序结构类型
    7) 解释器内部使用的内置类型
        * 代码对象
        * 帧对象
        * 跟踪对象
        * 生成器对象
        * 切片对象
        * Ellipsis对象
    8) 对象的行为与特殊方法
        * 创建与销毁
        * 字符串表示
        * 比较与排序
        * 类型检查
        * 属性访问
        * 属性包装与描述符
        * List与Dict方法
        * 迭代
        * 数学操作
        * 可调用接口
        * 上下文管理协议
        * 对象检查与dir
> ### **1.4 运算符与表达式**
    1) 操作(数字, List, String, Dict, Set)
    2) 增量赋值
    3) 运算符(点.运算符, 函数调用()运算符)
    4) 转换函数
    5) Boolean表达式与真值
    6) 对象的比较与身份
    7) 优先级
    8) 条件表达式
> ### **1.5 程序结构与控制流**
    1) 程序结构与执行
    2) 条件语句
    3) 循环与迭代
    4) 异常(内置异常，新定义异常)
    5) 上下文管理器与with语句
    6) assert与__debug__
> ### **1.6 函数与函数编程**
    1) 函数
    2) 参数传递与返回值
    3) 作用域
    4) 函数对象与闭包
    5) 装饰器
    6) 生成器与yield, 协程与yield
    7) 使用生成器和协程
    8) 列表包含
    9) 生成器表达式
    10) 声明式编程
    11) lambda运算符
    12) 递归
    13) 文档字符串
    14) 函数的属性
    15) eval(), exec(), compile()
> ### **1.7 类与面向对象编程**
    1) class与实例
    2) 范围规则
    3) 继承(多继承，与C#等的区别)
    4) 多态动态绑定与鸭子类型
    5) 静态方法与类方法
    6) 特性
    7) 描述符
    8) 数据封装与私有属性
    9) 对象内存管理
    10) 对象表示与属性绑定
    11) __slots__
    12) 运算符重载(与C++等的区别)
    13) 类型和类成员测试
    14) 抽象基类
    15) 元类(元编程)
    16) 类装饰器
> ### **1.8 模块、包与分发**
    1) 模块与import
    2) 从模块导入选定符号
    3) 以主程序的形式执行
    4) 模块的搜索路径
    5) 模块的加载与编译，重新加载和卸载
    6) 包
    7) 分发Python程序和库
    8) 安装第三方库
> ### **1.9 输入与输出**
    1) 命令行选项
    2) 环境变量
    3) 文件和文件对象
    4) 标准输入、输出和错误
    5) print语句和print函数
    6) 文件输出中的变量插入
    7) 生成输出
    8) Unicode字符串处理
    9) Unicode IO(数据编码和字符特性)
    10) 对象持久化和pickle模块
> ### **1.10 执行环境**
    1) 解释器选项与环境
    2) 交互式会话
    3) 启动Python应用程序
    4) 站点配置文件
    5) 用户站点包
    6) 启动新功能
    7) 程序终止
> ### **1.11 测试、调试、探查与调优**
    1) 文档字符串和doctest模块
    2) 单元测试与unittest模块
    3) 调试器与pdb模块(调试命令，命令行调试，配置调试器)
    4) 程序探查
    5) 调优与优化(计时测量，内存测量，反汇编，调优策略)


## **2. Python库**
> ### **2.1 内置函数和异常**
    1) 内置函数和类型
    2) 内置异常(异常基类，异常实例，预定义异常)
    3) 内置警告
    4) future_builtins
> ### **2.2 Python运行时服务**
    1) atexit(看看和C语言的区别)
    2) copy
    3) gc
    4) inspect
    5) marshal
    6) pickle
    7) SYS(变量和函数)
    8) traceback(和lua的区别)
    9) types
    10) warnings
    11) weakref(若引用，可以想想C++的实现)
> ### **2.3 数学运算**
    1) decimal(Decimal对象, Context对象, 函数和常量)
    2) fractions
    3) math
    4) numbers
    5) random(种子和初始化，随机整数，随机序列，实值随机分布)
> ### **2.4 数据结构、算法与代码简化**
    1) abc
    2) array(看看和C++的vector的区别)
    3) bitsect
    4) collections(deque和defaultdict，命名元组，抽象基类)
    5) contextlib
    6) functools
    7) heapq
    8) itertools
    9) operator
> ### **2.5 字符串和文本处理**
    1) odecs
        * 低级codecs接口
        * IO相关函数
        * 有用的常量
        * 标准编码
        * 注意事项
    2) re
        * 模式语法
        * 函数
        * 正则表达式对象
        * 匹配对下岗
        * 示例
        * 注意
    3) string
        * 常量
        * Formatter对象
        * Template字符串
        * 实用工具函数
    4) struct
        * 打包和解包
        * struct对象
        * 格式编码
        * 注意
    5) unicodedata
> ### **2.6 Python数据库访问**
    1) 关系数据库API规范
        * 连接
        * Cursor
        * 生成查询
        * 类型对象
        * 错误处理
        * 多线程
        * 将结果映射到字典
        * 数据库API扩展
    2) sqlite3模块
        * 模块级函数
        * 连接对象
        * 游标和基本操作
    3) DBM风格的数据库模块
    4) shelve模块
> ### **2.7 文本和目录处理**
    1) bz2
    2) filecmp
    3) fnmatch
    4) glob
    5) gzip
    6) shutil
    7) tarfile(异常和示例)
    8) tempfile
    9) zipfile
    10) zlib
> ### **2.8 操作系统服务**
    1) Commands模块
    2) ConfigParser、configparser
    3) datetime
        * data对象
        * time对象
        * datetime对象
        * timedelta对象
        * 涉及日期的数学计算
        * tzinfo对象
        * 日期与时间解析
    4) errno模块
    5) fcntl模块(看看和*nix中C的联系)
    6) io模块(基本IO，原始IO，缓存二进制IO，文本IO，open函数，抽象基类)
    7) logging模块
        * 日志记录级别(想想C++的实现)
        * 基本配置
        * Logger对象
        * 处理器对象
        * 消息格式化
        * 各种实用工具函数
        * 日志记录配置
        * 性能考虑
    8) mmap模块
    9) msvcrt模块
    10) optparse模块
    11) os模块
        (进程环境，文件创建与描述符，文件与目录，进程管理，系统配置，异常)
    12) os.path模块
    13) signal模块
    14) subprocess模块(想想C中的exec等等)
    15) time模块
    16) winreg模块
> ### **2.9 线程与并发性**
    1) 基本概念
    2) 并发编程与Python
    3) multiprocessing模块
        * 进程
        * 进程间通信
        * 进程池
        * 共享数据与同步
        * 托管对象
        * 连接
        * 各种实用工具函数
        * 多进程处理的一般建议
    4) threading模块
        * Thread对象
        * Timer对象
        * Lock对象
        * RLock
        * 信号量与有边界的信号量
        * 事件
        * 条件变量
        * 使用Lock
        * 线程终止与挂起
        * 实用工具函数
        * 全局解释器锁定
        * 使用线程编程
    5) queue和Queue模块
    6) 协程与微线程
> ### **2.10 网络编程与套接字**
    1) 网络编程基础
    2) asynchat模块
    3) asyncore模块
    4) select（高级功能，高级异步IO，异步联网的时机）
    5) socket(地址族，套接字类型，寻址，函数，异常)
    6) ssl
    7) SocketServer(处理程序，服务器，自定义服务器)
> ### **2.11 Internet应用程序编程**
    1) ftplib
    2) http包
        * http.client
        * http.server
        * http.cookies
        * http.cookiejar
    3) smtplib
    4) urllib
        * urllib.request
        * urllib.response
        * urllib.parse
        * urllib.error
        * urllib.robotparser
    5) xmlrpc
        * xmlrpc.client
        * xmlrpc.server
> ### **2.12 Web编程**
    1) cgi
    2) cgitb
    3) wsgiref(WSGI规范，wsgiref包)
    4) webbrowser
> ### **2.13 Internet数据处理和编码**
    1) base64
    2) binascii
    3) CSV
    4) email包（解析电子邮件，编写电子邮件）
    5) hashlib
    6) hmac
    7) HTMLParser
    8) json
    9) mimetypes
    10) quopri
    11) xml包
        * xml.dom.minidom
        * xml.etree.ElementTree
        * xml.sax
        * xml.sax.saxutils
> ### **2.14 其他库模块**
    1) Python服务
    2) 字符串处理
    3) 操作系统模块
    4) 网络和Internet数据处理
    5) 国际化
    6) 多媒体服务
