# **数据文件与持久性** #
***


## **1. 数据文件** ##
    1) 为应用儿创建数据文件, 可以使用lua的构造式, 每条数据记录表示为一个lua
       构造式
    2) Entry{<code>}, 以一个table作为参数来调用Entry 
        Entry{
          "Jon Bentley", 
          "More Programming Pearls", 
          "Addison-Wesley", 
          1990
        }
    3) Entry作为一个回调函数, 在dofile时为数据文件中的每个条目所调用
    4) 如文件不大, 可以使用名值对来表示每个字段
        Entry{
          author = "Jon Bentley", 
          title = "More Programming Pearls", 
          year = 1990, 
          publisher = "Addison-Wesley",
        }



## **2. 串行化** ##
    1) 经数据转换为一个字节流或字符流, 就可将其存储到一个文件中或通过网络连
       接发送出去
    2) 使用一种简单安全的方法来括住一个字符串, 就是以"%q"来使用string.format
       函数
       -- lua
       function serialize(o)
        if type(o) == 'number' then
          io.write(o)
        elseif type(o) == 'string' then
          io.write(string.format('%q', o))
        else
        end
       end
> ### **2.1 保存无环的table** ###
        -- lua
        function serialize(o)
          if type(o) == 'number' then
            io.write(o)
          elseif type(o) == 'string' then
            io.write(string.format('%q', o))
          elseif type(o) == 'table' then 
            io.write('{\n')
            for k, v in pairs(o) do 
              io.write(' ', k, ' = ')
              serialize(v)
              io.write(', \n')
            end
            io.write('}\n')
          else 
            error('cannot serialize a ' .. type(o))
          end
        end 
> ### **2.2 保存有环的table** ###
        -- lua
        function basic_serialize(o)
          if type(o) == 'number' then 
            return tostring(o)
          else 
            return string.format('%q', o)
          end
        end 
        function save(name, value, saved)
          saved = saved or {}
          io.write(name, ' = ')
          if type(value) == 'number' or type(value) == 'string' then
            io.write(basic_serialize(value), '\n')
          elseif type(value) == 'table' then
            if saved[value] then 
              io.write(saved[value], '\n')
            else 
              saved[value] = name 
              io.write('{}\n')
              for k, v in pairs(value) do 
                k = basic_serialize(k)
                local fname = stri.format('%s[%s]', name, k)
                save(fname, v, saved)
              end
            end 
          else 
            error('cannot save a ' .. type(value))
          end
        end
