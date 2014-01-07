# **资源管理** #
***




## **13: 以对象管理资源** ##
    (Use objects to manage resources.)
    1) 获得资源后立刻放进管理对象内
    2) 管理对象运用析构函数确保资源被释放
    3) 为防止资源泄露, 请使用RAII对象, 它们在构造函数中获得资源并在析构函数
       中释放资源
    4) 两个常被使用的RAII classes分别是tr1::shared_ptr和auto_ptr; 前者是较
       好的选择, 因为其copy行为比较直观; 若选择auto_ptr, 复制动作会使它指向
       null
