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





## **3. 链表**
    1) 每个节点以一个table来表示, 一个链接只是节点table中的一个字段, 该字段
       包含对其他字段table的引用;
          
          list = {next = list, value = v}
          local l = list
          while l do
            <l.value>
            l = l.next
          end
          end
        end
