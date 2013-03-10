# **模块与包** #
*** 

        require用于使用模块, module用于创建模块; 一个模块就是一个程序块, 可
    以通过require来加载, 然后就得到一个全局变量, 表示一个table(类似一个命名
    空间); 使用table来实现模块的优点在于可以像操作普通table那样来操作模块, 
    并能利用lua现有的功能来实现各种额外功能

## **1. require函数** ##
    1) 对require而言, 一个模块就是一段定义了一些值的代码
    2) require "<模块名>", 该调用会返回一个由模块函数组成的table, 且还会定义
       一个包含该table的全局变量
    3) 如果require为指定模块找到一个lua文件就通过loadfile来加载该文件, 如果
       找到的是一个C程序库, 就通过loadlib来加载; loadfile和loadlib只是加载了
       代码, 并没有运行; 为了运行代码, require会以模块名作为参数来调用这些代
       码


## **2. 编写模块的基本方法** ##
    1) 最简单的方法: 创建一个table, 并将所有需要到处的函数放到table中, 最后
       返回这个table, 例子如下:
        complex = {}

        function complex.new(r, i)
          return {r=r, i=i}
        end 

        complex.i = complex.new(0, 1)

        function complex.add(c1, c2)
          return complex.new(c1.r + c2.r, c1.i + c2.i)
        end 

        function complex.sub(c1, c2)
          return complex.new(c1.r - c2.r, c1.i - c2.i)
        end 

        function complex.mul(c1, c2)
          return complex.new(c1.r * c2.r - c1.i * c2.i, 
                             c1.r * c2.i + c1.i * c2.r)
        end 

        local function inv(c)
          local n = c.r ^ 2 + c.i ^ 2
          return complex.new(c.r / n, -c.i / n)
        end 

        function complex.div(c1, c2)
          return complex.mul(c1, inv(c2))
        end 

        return complex



## **3. 使用环境** ##
    1) 创建模块的基本方法的确定是, 它要求程序员投入一些额外的关注, 当访问同
       一个模块中的其他公共实体时, 必须限定其名称
    2) 参见书籍 ...


## **4. module函数** ##
    1) 编写一个模块时, 可以直接使用module来代替前面讲的设置
        module(...)
       其会创建一个新的table, 并将其赋予适当的全局变量和loaded table, 最后还
       会将这个table设置为主程序块的环境
    2) 使用了module之后, module不提供外部访问m 必须在调用它前为需要访问的外
       部函数或模块声明适当的局部变量
