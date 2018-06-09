# **PyDictObject**
***

原则上关联容器的设计总会关注索引效率，实现会基于设计良好的数据结构，与C++中的map类似，map是基于RBTree实现的，RBTree是一种平衡二叉树，搜索效率为O(log2N)；python中的dict由于需要在实现中被大量使用所以对搜索效率要求很苛刻（比如会通过PyDictObject建立执行python字节码的运行环境，存放变量名和变量值的元素对），因此采用了hash table，理论上的最优情况能提供O(1)的搜索效率；最新的C++中如`unordered_map`
