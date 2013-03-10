# **弱引用表** #
***
        弱引用就是一种会被垃圾收集器忽视的对象引用, 如果一个对象的所有引用都
    是弱引用, 那么lua久可以回收这个对象了, 并且还可以以某种形式来删除这些弱
    引用本身;
        有3中弱引用table: 具有弱引用key的table, 具有弱引用value的table, 同时
    具有两种弱引用的table; 不管是那种类型的弱引用table, 只要有一个key或value
    被回收了, 那么他们所在的整个条目都会从table中删除.
        lua只会回收弱引用table中的对象, 而像数字和布尔这样的值是不可回收的; 
    字符串和数字, 布尔一样, 不会从弱引用table中删除

## **1. 备忘录函数** ##
    1) 如一个服务器, 接受并处理lua代码, 如果频繁调用loadstring, 开销一定很大
       所以:
        local results = {}
        setmetatable(results, {__mode = 'v'}) -- 使value成为弱引用
        function mem_loadstring(s)
          local res = results[s]
          if res == nil then
            res = assert(loadstring(s))
            results[s] = res
          ens
          return res
        end 
       由于key总是字符串, 则可以使这个table成为完全弱引用
        setmetatable(results, {__mode = 'kv'})



## **2. 对象属性** ##
    1) 当对象是一个table时, 可以通过适当的key将数字那个存储在这个table中
    2) 具体见原书...


## **3. 回顾table的默认值** ##
    1) 方法一, 使用一个弱引用table, 通过它将每个table与其值关联起来
        local defaults = {}
        setmetatable(defaults, {__mode = 'k'})
        local mt = {__index = function(t)
            return defaults[t]
          end}
        function setDefault(t, d)
          defaults[t] = d
          setmetatable(t, mt)
        end
    2) 方法二, 对每种不同默认值使用不同的元表
        local metas = {}
        setmetatable(metas, {__mode = 'v'})
        function setDefault(t, d)
          local mt = metas[d]
          if mt == nil then 
            mt = {__index = function() return d end}
            metas[d] = mt
          end
          setmetatable(t, mt)
        end 
    3) 方法一需要为每个table的默认值使用内存, 方法二需要为每种不同的默认值使
       用一组内存;
       如果程序中有上千个table和一些默认值, 使用方法二;
       如果只有很少的table和共享几个公用的默认值, 使用方法一
