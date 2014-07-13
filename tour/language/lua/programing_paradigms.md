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
