# **数据结构**
***



## **1. 数组**
    1) 可以使用#来计算数组的长度
    2) 可以使用0, 1或其他任意值来作为数组的起始索引
    3) 习惯使用1作为数组的索引



## **2. 矩阵与多维数组**
    1) 数组的数组: 一个table中每个元素都是另一个table
        local mt = {}
        for i = 1, N do
          mt[i] = {}
          for j = 1, M do
            mt[i][j] = 0
          end
        end
    2) 将两个索引合并成一个索引
        local mt = {}
        for i = 1, N do
          for j = 1, M do
            mt[(i - 1) * M + j] = 0
          end
        end





## **3. 链表**
    1) 每个节点以一个table来表示, 一个链接只是节点table中的一个字段, 该字段
       包含对其他字段table的引用;
          
          list = {next = list, value = v}
          local l = list
          while l do
            <l.value>
            l = l.next
          end



## **4. 队列和双向队列**
    1) 可以使用table库的insert和remove来实现
    2) 使用两个索引, 分别用于收尾
        local Queue = {}
        function Queue.New()
          return {first=0, last=-1}
        end

        function Queue.PushFront(queue, value)
          local first = queue.first - 1
          queue.first = first
          queue[first] = value
        end

        function Queue.PushBack(queue, value)
          local last = queue.last + 1
          queue.last = last
          queue[last] = value
        end

        function Queue.PopFront(queue)
          local first = queue.first
          if first > queue.last then
            error('queue is empty')
          end
          local value = queue[first]
          queue[first] = nil
          queue.first = first + 1
          return value
        end

        function Queue.PopBack(queue)
          local last = queue.last
          if queue.last > last then
            error('queue is empty')
          end
          local value = queue[last]
          queue[last] = nil
          queue.last = last - 1
          return value
        end


## **5. 集合与无序组**
    1) 将集合元素作为索引放入一个table之中, 只需要该值来索引table, 并查看结
       果是否是nil;



## **6. 字符串缓存**
    1) 可以使用table作为字符串缓冲, 关键是使用table.concat将给定列表中的所有
       字符串连接起来, 并返回连接的结果;
          local t = {}
          for line in io.lines() do
            t[#t + 1] = line .. '\n'
          end
          local s = table.concat(t)
