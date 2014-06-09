# **Python基础知识**
***


## **1. 数据结构**
> ### **1.1 列表**
    * list.append(x)    追加新对象
    * list.extend(l)    在列表末尾追加另一个列表
    * list.insert(i,x)  将对象插入到列表中
    * list.remove(x)    从列表中移除x的第一个匹配选项
    * list.pop([i])     从列表中移除一个元素, 默认最后一个
    * list.index(x)     返回x的第一个匹配选项的索引
    * list.count(x)     统计某元素在列表中出现次数
    * list.sort()       对列表进行排序(会修改原列表),可传递一个比较函数
    * list.reverse()    反转列表
    * x = sorted(y)     对y排序, 返回排序结果, 不修改y
> ### **1.2 元组**
    就是一个不可改变的列表, 可以作为键
> ### **1.3 字符串**
    * str.find(sub[, start[, end]])
      在str中寻找子串sub, 返回sub所在位置最左端位置;
    * str.join(iterable)将字符串连接在一起
    * str.lower()       字符串小写
