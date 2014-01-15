# **继承与面向对象设计** #
***



## **32: 确定你的public继承塑膜出is-a关系** ##
    (Make sure public inheritance models "is-a".)
    1) "public继承"意味is-a; 适用于基类身上的每一件事情一定也适用于子类身上,
       因为每一个子类对象也都是一个基类对象



## **33: 避免遮掩继承而来的名称** ##
    (Avoid hiding inherited names.)
    1) 子类内的名称会遮掩基类内的名称, 在public继承下从来没人希望如此
    2) 为了让被遮掩的名称在见天日, 可以使用using声明式或转交函数
