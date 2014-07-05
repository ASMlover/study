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
