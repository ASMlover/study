# **Lua Language**
***



## **整体结构**
> ### **环境**
  * 解释器程序
  * lua的参数选项
  * setfenv修改环境

> ### **变量**
  * 命名
    - 词法规范
    - 关键字
    - 全局变量
  * 内置类型
    - nil
    - boolean  
      1) true  
      2) false和nil为假, 其余为真  
    - number
    - string  
      1) 是不可变的值  
      2) 使用''和""来界定  
      3) 转义字符序列  
      4) 字符串连接操作符'..'  
      5) '#'放在字符串前用于获取字符串长度
    - table  
      1) 实现了关联数组  
      2) table既不是值也不是变量, 而是对象  
      3) 程序仅仅有table的一个引用  
      4) 最简单的构造式{}  
      5) table永远是匿名的, 一个指向table的变量和table自身没有固定联系  
      6) 可以a.x或a[x]来访问, a.x等价于a['x'], a[x]中则是x变量的值
    - function  
      1) 当作值看待  
      2) lambda
    - userdata
    - thread
  * 作用域
    - 默认是全局变量
    - 使用local表示局部变量
  * 生存期

> ### **程序元素**
  * 表达式
    - 算术操作符
    - 关系操作符
    - 逻辑操作符
    - 字符串连接
    - 操作符优先级
    - table构造式
  * 语句
    - 赋值  
      1) 多重赋值(a, b = 1, 2)  
      2) 单个赋值
    - 局部变量与块(block)
    - 控制结构  
      1) if then else/ if then elseif then  
      2) while  
      3) repeat  
      4) for  
      5) 泛型for  
      6) break和return
    - 函数  
      1) 多重返回值  
      2) 参数(变长参数, 具名参数)  
      3) closure(闭包函数)  
      4) 局部函数(使用local)  
      5) 尾调用(一个函数在调用另一个函数之后再无其他操作)
    - 协同程序(coroutine)  
      1) 4种状态(挂起, 运行, 思维, 正常)  
      2) yield让运行中的协同程序挂起; resume唤醒挂起的协同程序  
      3) 非抢占式的多线程

> ### **数据结构**
  * 数组
    - 可通过#计算数组的大小
    - 下标从1开始
    - 矩阵  
      1) 一个table的每个元素是另一个table  
      2) 将两个索引合并成一个索引
  * 链表
  * 队列和双向队列
  * 集合
  * 字符串缓冲(table作为字符串缓冲, 使用concat)
  * 图

> ### **编程范式**
  * 元表
    - 算术类元方法
    - 关系类元方法
    - 库定义的元方法
    - table访问的元方法  
      1) `__index -> 查询`  
      2) `__newindex -> 更新`  
      3) `具有默认值的table, 通过__index来修改默认的nil`
  * 模块和包
    - require 加载一个模块
    - 编写模块  
      1) 创建一个table, 其中放入需导出的函数, 最后返回这个table  
      2) 将table直接赋值给package.loaded来消除return
    - 使用环境
    - module函数
    - 子模块与包
  * 面向对象编程
    - 类
    - 继承
    - 多重继承
    - 访问权限(私密性)
    - 单一方法
> ### **程序库**
  * 数学库
  * table库
    - 插入删除
    - 排序
    - 连接
  * 字符串库
    - 基础字符串函数
    - 模式匹配函数
      1) string.find  
      2) string.match  
      3) string.gsub  
      4) string.gmatch
    - 模式 
    - 捕获
    - 替换
    - 技巧
  * I/O库
    - 简单I/O模型
    - 完整I/O模型
  * 操作系统库
    - 日期和时间
    - 系统调用
  * 调试库
    - 自省机制 debug.getinfo  
      1) 访问局部变量debug.getlocal  
      2) 访问非局部的变量debug.getupvalue  
      3) 访问其他协同程序debug.traceback
    - 钩子  
      1) 触发钩子的事件"call", "return", "line", "count"
    - 性能剖析(profile)
> ### **扩展(同C交互)**
  * C API
    - API概述  
      1) lua.h -> 基础函数  
      2) lauxlib.h -> 辅助库函数
    - 栈  
      1) 压入元素  
      2) 查询元素(栈底索引为1, 栈顶元素为-1)  
      3) 普通栈操作函数
  * 扩展
    - 基础应用(如配置)
    - table操作
    - 调用Lua函数
  * Lua中调用C 
    - 注册C函数以便Lua调用
    - 注册C模块到Lua调用
  * 编写C函数的技术
    - `数组操作(lua_rawgeti/lua_rawseti)`
    - 字符串操作
    - 在C函数中保存状态
  * 用户自定义类型
    - userdata
    - 元表
    - 面向对象的访问
    - 数组访问
    - light userdata
> ### **高级**
  * 线程和状态
    - 多个线程  
      1) lua_newthread  
      2) lua_resume  
      3) lua_yield
    - Lua状态
  * 内存管理
