# **Python技巧**
***

## **1. 性能**
    前提是针对Python2.x
    1) 获取dict的keys或values以及items的时候应该使用iterkeys, itervalues,
       iteritems; 
       这样遍历的是一个迭代器而不用返回一个list然后再删除掉...
    2) 使用内建函数(它们非常块):
        input()
        int()
        isinstance()
        issubclass()
        iter()
        open()
        ord()
        pow()
        print()
        property()
    3) 使用python多重赋值, 交换变量:
        x, y = y, x
    4) 尽量使用局部变量
        检索局部变量比检索全局变量快, 应该避免使用'global'关键字
    5) 尽量使用'in':
        使用'in'关键字, 简洁而快速
        for key in sequence:
          ...
    6) 使用延迟加载:
        将'import'声明移入函数中, 仅在需要的时候导入; 虽然不能提高整体的性能
        但是可以帮助你更均衡的分配模块的加载时间
    7) 在无限循环的时候使用'while 1':
        <while 1>是一个单步运算, 可以提升性能
    8) 使用list comprehension:
        Python的解析器可以在循环中发现它是一个可预测的模式而被优化, 另外list
        comprehension更具有可读性;
    9) 使用xrange处理长序列:
        xrange在序列中每次调用只产生一个整数元素; 而range是返回一个完整的元
        素列表, 在使用完后再销毁;
    10) 使用python generator:
        可以节省内存和提高性能
            chunk = (1000 * i for i in xrange(1000))
            chunk.next()  # return 0
            chunk.next()  # return 1000
    11) 了解itertools模块, 该模块对迭代和组合是非常有效的;
    12) 学习bisect模块保持列表排序, 这是一个免费的二分查找实现和快速插入有序
        序列的工具;
    13) 理解python列表, 实际上是一个数组; 
    14) 使用dict和set测试成员:
        检查一个元素是在dict或set是否存在是非常快速的; 因为dict和set是使用
        hash表来实现的; 如果需要经常检查成员, 使用dict或set作为你的容器;
            l = ['a', 'b', 'c']
            'a' in l  => slower
            'a' in set(l) => faster
    15) 请熟悉Python源码(http://svn.python.org/view/python/trunk/Modules/)
