# **ZMQ** #
***

## **1. 说明** ##
    这里主要记录了zmq库使用的各种例子

## **2. REQ-REP模式(Request-Reply)** ##
    1) zeroMQ中的应答模式
    2) 不能同时发生多个数据
    3) 只能一次请求一次回答 a->b->a->b ...

## **3. PUB-SUB模式(Publish-Subscribe)** ##
    1) zeroMQ中的发布订阅模式
    2) 发布者和订阅者的关系是绝对的
    3) 发布者不能recv只能send
    4) 订阅者不能send只能recv
    5) PUB将消息发送给所有SUB, SUB只接收与自己名字匹配的消息
    6) zmq_setsockopt中的option_value虽然是一个void*指针, 但是传递进去的字符
       串其只会判断第一个字节中的值(因为测试的时候当传入类sub1, sub2的时候, 
       会接收到所有的信息)
