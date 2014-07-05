# **变量**
***


## **命名**
> ### **词法规范**
    1) 字母, 数字, 下划线, 数字不开头
    2) 避免使用下划线开头的变量, 或多个大写字母标识的变量
    3) _在lua中是哑元变量
> ### **关键字**
    and     break     do      else      elseif      end     false
    for     function  if      in        local       nil     not
    or      repeat    return  then      true        until   while
> ### **注释**
    1) '--' 开始一个行注释
    2) 以 '--[[' 开始直到 ']]' 结束表示一个块注释
> ### **全局变量**
    1) 不需要声明
    2) 未声明的变量的值是nil
    3) 需要删除某个全局变量的时候只需要将变量赋值为nil




## **内置类型**
    1) nil类型
       只有一个值nil, 全局变量的第一次赋值前的默认值为nil;
       nil表示一种"无效值";
    2) boolean类型
       只有false和true;
       false和nil为false, 其余值为true;
    3) number类型
       表示实数, 可以使用普通的写法, 也可以使用科学计数法;
    4) string类型
       采用8位编码, 可以将任意二进制数据存储到一个字符串中;
       转义字符:
          \a        响铃
          \b        退格
          \f        提供表格
          \n        换行
          \r        回车
          \t        水平tab 
          \v        垂直tab
          \\        反斜杠
          \"        双引号
          \'        单引号
       调用tostring或者数字与一个空字符串相连可转换一个数字为字符串;
       在字符串前放置#可以获取字符串的长度;
    5) table类型
       是一个关联数组, 可以通过除nil之外的任何值作为索引;
       table没有固定大小, 可以动态添加任意数量的元素到一个table中;
       table是一种动态分配的对象, 程序仅持有一个引用, lua不会暗中产生副本;
       数组通常以1作为索引起始, 用#获取长度;
    6) function类型
       函数可存储在变量中, 可以通过参数传递给其他函数, 也可以作为返回值;
    7) userdata和thread类型
       userdata可将任意C数据存储到lua变量中;
       thread即是coroutine;
