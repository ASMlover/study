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
    * str.replace(old, new[, count])
      返回某字符串的所有匹配项均被替换之后得到的字符串
    * str.split([sep[, maxsplit]])
      根据sep将字符串分割成序列
    * str.strip([chars])
      去除左右两边的含有chars的字符, 默认是空格, 不包含中间
    * str.translate(table[, deletechars])
      替换字符串的某些部分, 要求一个替换后的字符表, 和一个被替换的字符表
      'read this short text'.translate(None, 'aeiou')
      'rd ths shrt txt'
> ### **1.4 字典**
    * 字典格式化
      book = {'bookname1' : 'author1', 'bookname2' : 'author2'}
      print '<<bookname1>> author is : %(bookname1)s' % book
    * dict.clear()      清除字典中所有项
    * dict.copy()       拷贝一个具相同键-值对的字典(浅拷贝)值是可变的才可改
    * dict.fromkeys(seq[, value])
      用给定的序列元素作为key创建一个字典, 值默认为None;
    * dict.get(key[, default])
      如果字典中有key, 返回key对应的值, 否则返回default(默认None);
    * dict.has_key(key) 检查字典中是否有给定的key;
    * dict.items()/dict.iteritems()
      items以列表形式返回字典的键值对(元组形式), 没有特殊的顺序;
      iteritems返回一个迭代器对象, 每次迭代为键值对(元组);
    * dict.keys()/dict.iterkeys()
      keys以列表形式返回字典中的键; iterkeys返回针对键的迭代器;
    * dict.pop(key[, default])
      获取key对应的值, 并将这个键-值对删除; 如果提供default, 没有key返回默认
    * dict.popitem()    弹出一个随机项
    * dict.setdefault(key[, default])
      如何key存在返回它的值, 如果不存在设置key的值为default, 默认为None;
    * dict.update([other])
      利用一个字典更新另一个字典, 新的项添加到dict, 旧的项被other替换;
    * dict.values()/dict.itervalues()
      values以列表形式返回字典中的值, itervalues返回字典中值的迭代器;
