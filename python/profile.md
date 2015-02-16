# **Python性能优化**
***

## **1. 定位性能瓶颈**
    1) 可以使用profile, cProfile, hotshot来定位性能瓶颈
    2) 使用方法:
        $ python -m profile test.py
    3) 显示结果:
        * filename:lineno(function): 文件名: 第几行(函数名字)
        * ncalls: 调用了多少次
        * tottime: 总共花费了多少时间
        * percall: 平均每次调用花费的时间 (tottime / ncalls)
        * cumtime: 它所有内部函数花费的时间
        * percall: (cumtime / ncalls)

## **2. 小技巧**
    1) 使用''.join(string_list)替换+或+=
    2) 在2.x版本使用xrange替换range
    3) 列表推导式比for循环快, while循环最慢, while使用的是外部计数
    4) 使用del删除使用后的对象:
        * python自己会做, 但需要额外的垃圾回收模块
        * 写一个__del__ magic
        * 直接使用del删除
