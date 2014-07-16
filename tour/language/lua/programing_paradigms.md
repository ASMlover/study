# **编程范式**
***



## **1. 元表**
  * Lua在创建table的时候不会创建元表; 可以使用setmetatable来设置或修改元表;
  * Lua中只能设置table的元表, 若要设置其他类型值的元表, 需要使用C来完成;
  * 所有的字符串都有一个默认元表;


> ### **1.1 算术类的元方法**
    __add -> 加
    __mul -> 乘
    __sub -> 减
    __div -> 除
    __unm -> 相反数
    __mod -> 取模
    __pow -> 幂
    __concat -> 连接操作符
    1) 查找元表:
       如果第一个值有元表, 查找对应的元方法, 与第二个值无关; 如果第二个值有
       对应的元方法, 以第二个值为准; 如果两个值都没有对应的元方法, 引发错误
> ### **1.2 关系类的元方法**
    __eq -> 等于
    __lt -> 小于
    __le -> 小于等于
    1) 关系类的元方法不能应用于混合类型
    2) 对混合类型而言, 关系类元方法就是模拟这些操作在Lua中普通的行为
    3) 等于比较用于不会引发错误, 如果两个对象拥有不同的元方法, 那等于操作不
       会调用任何一个元方法, 直接返回false
> ### **1.3 库定义的元方法**
    1) 格式化任意值时, tostring会检查该值是否有一个__tostring的元方法, 如有
       tostring就用该值作为参数来调用这个元方法
    2) setmetatable和getmetatable会用到元表的一个字段, 用于保护元表; 
       如想要用户不能看也不能修改元表, 那就需要使用__metatable, 设置了该元方
       法后getmetatable就会返回该值, 而setmetatable会引发一个错误;
> ### **1.4 table访问的元方法**
    1) __index
       * 访问table一不存在字段会让解释器查找__index元方法, 如果没这元方法, 
         返回nil;
       * 如不想在访问一个table时涉及__index, 使用rawget(t,i)对table进行一个
         原始的访问;
    2) __newindex
       * __newindex用于table的更新, __index用于table的查询;
       * 对一个table不存在的索引赋值时解释器会查找__newindex元方法; 如果有该
         元方法就调用__newindex而不进行赋值; 如果__newindex是一个table, 就在
         table中进行赋值, 而不是原来的table;
       * 使用rawset(t,k,v)可以绕过元方法__newindex




## **2. 模块和包**
> ### **2.1 require函数**
    1) 调用 require '<模块名>', 返回一个由模块函数组成的table, 还会定义一些
       包含该table的全局变量;
    2) require行为
          function require(name)
            if not package.loaded[name] then
              local loader = findloader(name)
              if loader == nil then
                error('unable to load module' .. name)
              end
              package.loaded[name] = true
              local res = loader(name)
              if res ~= nil then
                package.loaded[name] = res
              end
            end

            return package.loaded[name]
          end
    3) require的路径每一项以;隔开, require会以模块名来替换每个?, 路径如下:
        ?;?.lua;/usr/local/lua/?/?.lua 
    4) require用于搜索lua文件的路径存放在变量package.path中, lua启动后会以
       环境变量LUA_PATH来初始化这个变量;
    5) package.cpath用于存放C库, 如下:
        ./?.so;/usr/local/lib/lua/5.1/?.so 
        .\?.dll;D:\lua\dll\?.dll
       找到C库后require会调用package.loadlib来加载;
> ### **2.2 编写模块的方法**
    1) 创建一个table, 并将所有需要导入的函数放入其中, 最后返回这个table;
    2) 因为require会将模块名作为参数传递给模块, 所以可以避免写模块名:
          local module_name = '...'
          local M = {}
          _G[module_name] = M
          ...
    3) 将模块table直接赋予package.loaded就可以避免写return语句了:
          local module_name = '...'
          local M = {}
          _G[module_name] = M
          package.loaded[module_name] = M
          ...
    4) 如果一个模块没有返回值的话, require就会返回package.loaded[modname]的
       当前值;
> ### **2.3 使用环境**
    1) 思想就是让模块的主程序块有一个独占的环境, 这样它的所有函数都在这个模
       块, 而且它的所有全局变量也都记录在这个table中;
       模块只需要将这个table赋予模块名和package.loaded;
          local module_name = '...'
          local M = {}
          _G[module_name] = M
          package.loaded[module_name] = M
          setfenv(1, M)
          这样当再在这个模块中添加函数的时候就会成为<module_name>.<function>
          function add(c1, c2)
            ...
          end
          add就是模块的一个函数
    2) 当使用环境之后就不可以再使用全局的变量了, 最简单的方法就是使用继承来
       解决这个问题:
          local module_name = '...'
          local M = {}
          _G[module_name] = M
          package.loaded[module_name] = M
          setmetatable(M, {__index = _G})
          setfenv(1, M)
    3) 另一种方法是声明一个局部变量, 用来保存对旧环境的访问:
          ...
          local _G = _G
          setfenv(1, M)
          ...
    4) 更正规的方法就是将那些需要使用到的函数或模块声明为局部变量:
          ...
          local sqart = math.sqart
          local io = io
          ...
          setfenv(1, M)
          ...
> ### **2.4 module函数**
    1) 在创建模块的时候可以直接使用module函数来替换前面创建模块的代码;
    2) module会创建一个新的table, 并将其赋予适当的全局变量和loaded table; 最
       后还会将这个table设为主程序块的环境;
    3) module不提供外部访问, 必须在调用之前, 为需要访问的外部函数或模块声明
       适当的局部变量, 当然可以通过继承来实现; 
       在调用module的时候加一个package.seeall选项就可以实现继承;
          module(..., package.seeall)
> ### **2.5 子模块与包**
    1) 支持具有层级性的模块名, 可以用一个点来分隔名称中的层级;
    2) require一个带点的模块时, 会用原始的模块名作为key来查询package.loaded
       和package.preload; 模块名中的点在搜索中无任何意义;




## **3. 面向对象编程**
    Lua中使用冒号, 其作用是在一个方法定义中添加一个额外的隐藏参数, 以及在一
    个方法调用中添加一个额外的实参;
> ### **3.1 类**
    1) 如果要实现Basic是Child的基类, 只需要使用setmetatable来设置__index即可
       setmetatable(Child, {__index = Basic})
       这样Child就会在Basic中查找所有它没有的操作;
> ### **3.2 继承**
        Basic = {}
        function Basic:New(o)
          o = o or {}
          setmetatable(o, self)
          self.__index = self
          return o
        end
        Child = Basic:New()
        local ins = Child:New()
        从而可以实现继承, 可以重载Basic的函数, 这样就不会查找Basic的函数了;
> ### **3.3 多重继承**
    1) 关键在于用一个函数作为__index元字段;
        local function Search(k, plist)
          for i = 1, #plist do
            local v = plist[i][k]
            if v then 
              return v
            end
          end
        end
        -- 创建多继承的新类
        function CreateClass(...)
          local c = {}
          local parents = {...}
          setmetatable(c, {__index = function(t, k)
            return Search(k, parents)
          end})
          c.__index = c
          function c:New(o)
            o = o or {}
            setmetatable(o, c)
            return o
          end
          return c
        end
    2) 由于查找的路径较长, 多重继承的性能不如单一继承, 可以使用以下方法来解
       决:
          setmetatable(c, {__index = function(t, k)
            local v = Search(k, parents)
            t[k] = v
            return v
          end})
       本质就是将继承的方法复制到子类中;
> ### **3.4 私密性**
        function NewAccount(init_balance)
          local self = {balance = init_balance}
          local WithDraw = function(v)
            self.balance = self.balance - v
          end
          local Deposit = function(v)
            self.balance = self.balance + v
          end
          local GetBalance = function()
            return self.balance
          end
          return {WithDraw = WithDraw, 
                  Deposit = Deposit, 
                  GetBalance = GetBalance}
        end
> ### **3.5 单一方法做法**
    当对象只有一个方法的时候, 可以不用创建接口table, 但需要将这个单独的方法
    作为对象来返回;
    单一对象方法还可以使用类调度的方法来实现, 传递入参数来返回对应的值:
        function NewObject(value)
          return function(action, v)
            if action == 'get' then
              return value
            elseif action == 'set' then
              value = v
            else 
              error('invalid action')
            end
          end
        end
    使用该方法可以完全将对象实现方式隐藏在内部, 也比用传统的对象table方式更
    加高效, 但是不能实现继承;



## **4. 若引用table**
    * 就是一种会被垃圾收集器忽视的对象引用, 如果一个对象的所有引用都是弱引用
      那么Lua就可以回收这个对象了;
    * 3中若引用table:
      * 具有若引用key的table;
      * 具有若引用value的table;
      * 同时具有若引用key和若引用value的table;
    * 无论哪种若引用table, 只要有一个key或value被回收, 那么它们所在的整个条
      目都会从table中删除;
    * table的若引用类型是通过元表的__mode来决定的, 值是一个字符串; 
      包含'k' -> key是若引用
      包含'v' -> value是若引用
    * lua只会回收弱引用table中的对象, 数字/bool值/字符串是不可回收的;
