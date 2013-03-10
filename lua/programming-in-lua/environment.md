# **环境** #
***

        lua将所有全局变量保存在一个table中, 这个table就是环境; 其优点在于, 
    一, 不需再为全局变量创建一种新的数据结构, 简化了lua内部实现; 二, 可以像
    其他table一样操作这个table。lua环境变量table存在全局变量_G中。

## **1. 具有动态名字的全局变量** ##
    1) 当操作一个全局变量时, 它的名称却存储在另一个变量中, 或者需要通过运行
       时的计算才能得到;
       可采取如下方式:
        value = _G[varname]


## **2. 全局变量声明** ##
    1) 不需要声明就可以直接使用
    2) lua将全局变量存放在一个普通的table中, 则可以通过元表来改变其访问全局
       变量时的行为;
       检测所有对全局table中不存在的key访问, 方法如下:
        setmetatable(_G, {
          __newindex = function(_, n)
            error('attempt to write to undeclared variable' .. n, 2)
          end, 
          __index = function(_, n)
            error('attempt to read undeclared variable' .. n, 2)
          end, 
        })
       指定该代码后, 所有对全局table中不存在的key的访问都会引发一个错误;
       声明新的变量时, 可以使用rawset来绕过元表:
        function declare(name, initval)
          rawset(_G, name, initval or false)
        end
    3) 一种简单的方法就是只允许在主程序块中对全局变量进行赋值, 当遇到声明的
       时候就只需要检查此赋值是否在主程序中; 
       debug.getinfo(2, 'S')返回一个table, 其中的what字段表示调用该元方法的
       函数是主程序块还是普通的lua函数, 又或者是C函数;
       这样上面的__newindex可以改写为如下:
        __newindex = function(t, n, v)
          local w = debug.getinfo(2, 'S').what
          if w ~= 'main' and w ~= 'C' then
            error('attempt to write to undeclared variable' .. n, 2)
          end
          rawset(t, n, v)
        end
    4) 由于不允许全局变量具有nil值, 因为具有nil值的全局变量被认为是从未声明
       过的, 要想实现x = nil可以起到声明全局变量的作用, 可以使用一下方法:
        local declared_names = {}
        setmetatable(_G, {
          __newindex = function(t, n, v)
            if not declared_names[n] then 
              local w = debug.getinfo(2, 'S').what
              if w ~= 'main' and w ~= 'C' then
                error('attempt to write to undeclared variable' .. n, 2)
              end
              declared_names[n] = true
            end 
            rawset(t, n, v)
          end, 
          __index = function(_, n)
            if not declared_names[n] then 
              error('attempt to read undeclared variable' .. n, 2)
            else 
              return nil
            end
          end, 
        })



## **3. 非全局的环境** ##
    1) 可以使用setfenv来改变一个函数的环境, 其参数是一个函数呵呵一个新的环境
       table; 第一个参数除了可以指定为函数本身, 还可以指定为一个数字, 以表示
       当前函数调用栈中的层数(1表示当前函数, 2表示调用当前函数的函数)
    2) 一旦改变了环境, 所有全局的访问都会使用新的table, 如果新的table是空的
       那就会丢失所有的全局变量, 包括_G
        a = 1
        setfenv(1, {g = _G})
       这样访问g.a才会有效, 此时访问全局的g就会得到原来的环境
    3) 另一种组装新环境的方法是使用继承
        a = 1
        local newgt = {}  -- 创建新环境
        setmetatable(newgt, {__index = _G})
        setfenv(1, newgt)
        print(a)          -- > 1
       新环境从原环境中继承了print和a, 但是任何赋值都发生在新的table中
