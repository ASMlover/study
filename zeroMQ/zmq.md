# **ZMQ** #
***

## **1. 说明** ##
    这里主要记录了zmq库使用的各种例子

## **2. REQ-REP模式(Request-Reply)** ##
    见图req-rep.png
    1) zeroMQ中的应答模式
    2) 不能同时发生多个数据
    3) 只能一次请求一次回答 a->b->a->b ...

## **3. PUB-SUB模式(Publish-Subscribe)** ##
    见图pub-sub.png
    1) zeroMQ中的发布订阅模式
    2) 发布者和订阅者的关系是绝对的
    3) 发布者不能recv只能send
    4) 订阅者不能send只能recv
    5) PUB将消息发送给所有SUB, SUB只接收与自己名字匹配的消息
    6) zmq_setsockopt中的option_value虽然是一个void*指针, 但是传递进去的字符
       串其只会判断第一个字节中的值(因为测试的时候当传入类sub1, sub2的时候, 
       会接收到所有的信息)
    7) zmq_setsockopt的opvallen参数则表示了option_value设置的长度, 判断的时
       候可以根据此依据来判断名字长度

## **4. PUSH-PULL模式** ##
    见图push-pull.png
    多对多的模式请见图push-pull2.png
    1) 分为3个部分
       任务发布 -> 任务执行 -> 工作结果采集
       sender -> worker -> sinker
    2) 该模式和pub/sub模式类似, 都是单向的, PUSH发送, PULL接收
    3) 在没有worker的时候, sender的消息是不会消耗的(由发布进程维护)
    4) 多个worker消费的是同一列信息, 如果worker1得到消息1, 那么worker2将不会
       得到该消息了

## **5. 正确的使用Context** ##
    1) 0MQ总是从一个Context开始的
    2) 程序中应该从zmq_ctx_new开始, 然后以zmq_ctx_destroy结束
    3) 如果使用了fork, 那么每个进程有它自己的Context
    4) 如果在fork之前使用了zmq_ctx_new, 那么每个子进程都将有自己的Context

## **6. 干净的退出** ##
    1) 总是close一个你使用过的消息(zmq_msg_close)
    2) 如果你打开并关闭了很多(大量)的socket, 那就说明你需要重新设计你的程序
    3) 退出程序时, 关闭你的socket并调用zmq_ctx_destroy

## **7. 0MQ内置的绑定对** ##
    1) PUB - SUB
    2) REQ - REP
    3) REQ - ROUTER
    4) DEALER - REP 
    5) DEALER - ROUTER
    6) DEALER - DEALER 
    7) ROUTER - ROUTER
    8) PUSH - PULL 
    9) PAIR - PAIR
    10) XPUB - XSUB
