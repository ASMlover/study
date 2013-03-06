# **Statement(语句)** #
***


## **1. 赋值** ##
    1) 修改一个变量或table中字段的值
    2) 允许多重赋值, 可将多个值赋予多个变量
       a, b = 10, 20
       若值的个数少于变量格式, 那多余的变量会被赋值为nil


## **2. 局部变量与块(block)** ##
    1) 通过local来创建局部变量
    2) 一个块是一个控制结构的执行体, 或者是一个函数的执行体再或是一个程序块
    3) 交互模式中的每输入一行的内容自身就是一个程序块
    4) 可以使用do-end关键字显示界定一个块
    5) 尽可能地使用局部变量(避免搞乱全局变量, 访问局部变量更快)
    6) 缩短变量的作用于有助于提高代码的可读性


## **3. 控制结构** ##
    1) if then else
        if <exp> then 
          <stat>
        elseif <exp>
          <stat>
        else 
          <stat>
        end 
    2) while
        while <exp> do 
          <stat>
        end 
    3) repeat
        repeat 
          <stat>
        until <exp> 
    4) 数字型for (numeric for)
        for var = exp1, exp2, exp3 do 
          <stat>
        end 
        var从exp1到exp2, 且以exp3为步长, 默认为1 
    5) 泛型for (generic for)
        使用一个迭代器函数来遍历所有值
        for k, v in pairs(a) 
          <stat>
        end 
        k为索引值, v为该索引的数组元素值
    6) 泛型for和数字for有两个相同点: 循环变量是循环体的局部变量, 绝不应该对
       循环变量作任何赋值
    7) break与return
          都用于跳出当前的块;
          break用于结束一个循环, 它只会跳出包含它的那个内部循环, 而不会改变
        外层的循环;
          return语句用于从一个函数中返回结果, 或简单结束一个函数的执行;
          break或return只能是一个块的最后一条语句;
