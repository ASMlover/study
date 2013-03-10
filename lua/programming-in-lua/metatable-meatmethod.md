# **元表与元方法** #
***

## **1. 算术类的元方法** ##
    1) 在元表中, 每种算术操作符都有对应的字段名, __add(加), __mul(交集/乘), 
       __sub(减法), __div(除法), __unm(相反数), __mod(取模), __pow(乘幂), 还
       有__concat(描述连接操作符的行为)
        -- lua demo
        Set = {}

        function Set.new(l)
          local set = {}
          for _, v in pairs(l) do 
            set[v] = true
          end
          return set
        end 

        function Set.union(a, b)
          local res = Set.new({})
          for k in pairs(a) do 
            res[k] = true
          end 
          for k in pairs(b) do 
            res[k] = true
          end 
          return res 
        end 
        
        function Set.tostring(set)
          local l = {}
          for e in pairs(set) do
            l[#l + 1] = e
          end 
          return "{" .. table.concat(l, ", ") .. "}"
        end

        <1> 创建一个常规table, 准备用作集合的元素
            local mt = {}
        <2> 修改Set.new函数, 将mt设置为当前所创建table的元表
            function Set.new(l)
              local set = {}
              setmetatable(set, mt)
              for _, v in pairs(l) do 
                set[v] = true
              end 
              return set
            end 
        <3> 将元方法加入元表
            mt.__add = Set.union 



## **2. 关系类的元方法** ##
    1) 元表可以指定关系操作符的含义, 元方法为__eq(等于), __lt(小于), __le(小
       于等于); a ~= b会转化为not(a == b), a > b会转化为b < a, a >= b会转化 
       为b <= a 
       例子延续前面的如下:
        -- lua 
        mt.__le = function(a, b)
          for k in pairs(a) do 
            if not b[k] then 
              return false
            end
          end 
          return true
        end 
        mt.__lt = function(a, b)
          return a <= b and not (b <= a)
        end 
        mt.__eq = function(a, b)
          return a <= b and b <= a
        end
    2) 关系类元方法不能应用于混合的类型



## **3. 库定义的元方法** ##
    1) 函数print总是调用tostring来格式化其输出, 当格式化任意值时, tostring会
       检查该值是否有一个__tostring的元方法; 有, tostring就用该值作为参数来
       调用这个元方法 
        mt.__tostring = Set.tostring 
    2) setmetatable和getmetatable也会用到元表中的一个字段, 用于保护元表; 如
       想要保护集合的元表, 使用户既不能看也不能修改集合的元表, 那么久需要用
       到字段__metatable; 设置该字段后, getmetatable会返回这个字段的值, 而
       setmetatable则会引发一个错误
        mt.__metatable = "not your business"


## **4. table访问的元方法** ##
> ### **4.1 __index元方法** ###
    1) 访问一个table中不存在的字段, 得到nil
    2) 实际这些访问会促使解释器去查找一个__index的元方法, 如果没有这个元方法
       结果就是nil
        Window = {}
        Window.prototype = {x=0, y=0, width=100, height=100}
        Window.mt = {}
        -- 声明构造函数
        function Window.new(o)
          setmetatable(o, Window.mt)
          return o
        end 
        --
        -- 定义__index元方法
        Window.mt.__index = function(table, key)
          return Window.prototype[key]
        end
        --
        --
        -- 应用
        w = Window.new({x=10, y=20})
        print(w.width)      --> 100
    3) lua中__index元方法用于继承是很普遍的方法
    4) __index不一定必须是函数, 还可以是一个table 
       当__index是一个函数时, lua以table和不存在的key作为参数来调用该函数; 
       当__index是table时, lua就以相同的方法来重新访问这个table;
        Window.mt.__index = Window.prototype 
    5) 将一个table作为__index是一种快捷的, 实现单一继承的方式; 虽然函数作为
       __index相对开销大, 但是更灵活
    6) 如果不想在访问一个table时涉及它的__index, 可以使用rawget来访问
> ### **4.2 __newindex元方法** ###
    1) 与__index类似, 但是__index用于table的查询, __newindex用于table的更新
    2) 当对一个table中不存在的所有赋值就会查找__newindex元方法, 如果有这个元
       方法, 解释器就调用它, 而不是执行赋值
> ### **4.3 具有默认值的table** ###
    1) 常规table中的任何字段默认都是nil
    2) 可以为元表设置一个默认值
        -- lua 
        local key = {}
        local mt = {__index = function(t) return t[key] end}
        function setDefault(t, d)
          t[key] =  d
          setmetatable(t, mt)
        end 
        这样调用setDefault之后就可以设置一个你想的默认值
> ### **4.4 跟踪table的访问** ###
    1) __index和__newindex都是在table中没有所需访问的index时才发挥作用
    2) 为了监视一个table的所有访问, 就应该为真真的table创建一个代理, 这个代
       理就是一个空的table 
        local index = {}
        local mt = {
          __index = function(t, k)
            print('*access to element' .. tostring(k))
            return t[index][k]    -- 访问原来的table
          end, 
          __newindex = function(t, k, v)
            print('*update of element'..tostring(k)..'to'..tostring(v))
            t[index][k] = v       -- 更新原来的table
          end
        }
        function track(t)
          local proxy = {}
          proxy[index] = t
          setmetatable(proxy, mt)
          return proxy
        end
        这样若有监视table t, 只需要执行t = track(t)
> ### **4.5 只读的table** ###
    1) 只需要跟踪所有对table的更新操作并触发一个错误即可
        -- lua 
        function readOnly(t)
          local proxy = {}
          local mt = {
            __index = t, 
            __newindex = function(t, k, v)
              error("attempt to update a read-only table", 2)
            end
          }
          setmetatable(proxy, mt)
          return proxy
        end 
        这样就可以使用readOnly来创建一个只读的table了
        t = readOnly({1, 2, 3, 4})
        t[1] = 33 --> 触发只读错误
