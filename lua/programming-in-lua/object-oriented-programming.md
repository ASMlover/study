# **面向对象编程** #
***

        table就是一种对象, table与对象一样可以拥有状态; table与对象一样拥有
    一个独立于其值的标示(一个self); table与对象一样具有独立于创建者和创建地
    的声明周期。
        Account = {balance = 0}
        function Account.withdraw(self, v)
          self.balance = self.balance - v
        end
        这样就可以使用如下方式来使用Account了:
        a = Account
        Account = nil
        a.withdraw(a, 100)

        在lua中使用冒号就可以隐藏self参数了:
        function Account:withdraw(v)
          self.balance = self.balance - v
        end 
        调用的时候也可以修改为:
        a:withdraw(100) 

        现将例子修改如下:
        Account = {balance = 0, 
          withdraw = function(self, v)
            self.balance = self.balance - v
          end
        }
        function Account:deposit(v)
          self.balance = self.balance + v
        end


## **1. 类** ##
    1) lua中没有类的概念, 每个对象只能自定义行为和形态
       前面的例子可以修改如下:
        function Account:new(o)
          o = o or {}
          setmetatable(o, self)
          self.__index = self 
          return o
        end 



## **2. 继承** ##
        假设基类:
        Account = {balance = 0}

        function Account:new(o)
          o = o or {}
          setmetatable(o, self)
          self.__index = self
          return o
        end 

        function Account:deposit(v)
          self.balance = self.balance + v
        end 

        function Account:withdraw(v)
          if v > self.balance then 
            error("insufficient funds")
          end 
          self.balance = self.balance - v
        end 
    1) 可以使用创建一个Account类似的方法继承一个对象
        SpecialAccount = Account:new()
        s = SpecialAccount:new({limit = 0})
        s就是一个SpecialAccount(继承自Account)的实例
    2) 继承的对象还可以重写从基类继承的方法
        function SpecialAccount:withdraw(v)
          if v - self.balance >= self:getLimit() then 
            error("insufficient funds")
          end 
          self.balance = self.balance - v
        end 

        function SpecialAccount:getLimit()
          return self.limit or 0
        end 


## **3. 多重继承** ##
    1) 多重继承是在__index元方法中完成的
        local function search(k, plist)
          for i = 1, #plist do
            local v = plist[i][k]
            if v then 
              return v
            end
          end
        end 

        function createClass(..)
          local c = {}    -- 新类
          local parents = {...}

          -- 类在其父类列表中的搜索方法
          setmetatable(c, {__index = function(t, k)
              return search(k, parents)
            end})

          -- 将'c'作为其实例的元表
          c.__index = c 

          -- 为新类定义一个新构造函数
          function c:new(o)
            o = o or {}
            setmetatable(o, c)
            return o
          end 

          return c    -- 返回新类
        end 

        新定义另一个基类
        Named = {}
        function Named:getName()
          return self.name
        end 

        function Named:setName(name)
          self.name = name
        end 

        则创建一个新类从Account和Named继承如下:
        NamedAccount = createClass(Account, Named)


## **4. 私密性** ##
    1) lua没有提供私密性机制
    2) 但可以使用其他方法来实现对象, 从而获得对象的访问控制
    3) 通过两个table来表示一个对象, 一个table原来保存对象的状态, 另一个用于
       对象的操作(接口); 对象本身是通过第二个table来访问的
        function newAccount(initBalance)
          local self = {balance = initBalance}

          local withdraw = function(v)
            self.balance = self.balance - v
          end

          local deposit = function(v)
            self.balance = self.balance + v
          end 

          local getBalance = function()
            return self.balance
          end

          return {
            withdraw = withdraw, 
            deposit = deposit, 
            getBalance = getBalance
          }
        end



## **5. 单一方法做法** ##
    1) 当一个对象只有一个方法时, 可以不用创建接口table, 但要将这个单独的方法
       作为对象表示来返回
        function newObject(value)
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

       其使用如下:
        d = newObject(0)
        print(d('get'))
    2) 每个对象都用一个closure, 比都用一个table更高效
    3) 无法实现继承
    4) 拥有了完全的私密性控制
