# **Deep into Function(深入函数)** #
***


## **1. closure(闭包函数)** ##
    1) 如果将一个函数写在另一个函数内部, 那么内部函数可以访问外部函数的局部
       变量, 称为"词法域"
       -- lua
       function sort_by_grade(names, grades)
        table.sort(names, function (n1, n1)
            return grades[n1] > grades[n2]
          end)
       end
    2) 一个closure就是一个函数加上这个函数所需要访问的所有"非局部的变量"
    3) 可以重新定义已经预定义了的函数


## **2. 非全局函数** ##
    1) 可以将函数存储在table表中
        如, 常规做法:
          lib = {}
          lib.add = function(x, y) return x + y end 
        构造式做法:
          lib = {
            add = function(x, y) return x + y end
          }
        也可使用下面的方法:
          lib = {}
          function lib.add(x, y) return x + y end 
    2) 只要将一个函数存储到一个局部变量中, 就可以得到一个局部函数
    3) 定义local函数还可以使用
          local function foo(<参数>)
            <函数体>
          end

          local foo = function(<参数>)
            <函数体>
          end 

          -- 下面的方式可以解决递归函数调用的问题
          local foo
          foo = function(<参数>)
            <函数体>
          end 


## **3. 正确的尾调用** ##
    1) lua支持尾调用消除
    2) 尾调用类似goto的函数调用, 当一个函数调用是另一个函数的最后一个动作的
       时候, 这个调用才算一个尾调用
    3) 在进行尾调用的时候不耗费任何空间, 这种实现称为尾调用消除
    4) 要想受益于尾调用消除, 必须确定当前调用是一条尾调用(一个函数在调用完另
       一个函数之后, 是否就无其他事情需要做了)
    5) 只有"return <func>(<args>)"这样的调用形式才算是一条尾调用
    6) lua中的尾调用类似于直接goto到另一个函数了, 所以不会造成栈空间溢出
    7) 尾调用的时候, 函数参数只有不是一个函数就不会造成栈溢出
