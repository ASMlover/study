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

## **8. 0MQ处理多个Sockets** ##
> ### **8.1 传统的做法** ###
    传统的做法是在主循环中:
    1) 等待对应socket的消息(recv)
    2) 处理接收到的消息
    3) 循环
> ### **8.2 高效优雅的做法** ###
    1) 可以使用0MQ的zmq_poll
    2) 有消息来的时候自动通知对应的socket 
    3) 对应的socket接收消息并处理
> ### **8.3 zmq_poll对应的消息结构** ###
        typedef struct {
          void* socket;   // 0MQ socket to poll on
          int   fd;       // OR, native file handle to poll on
          short events;   // Events to poll on
          short revents;  // Events returned after poll
        } zmq_pollitem_t;
> ### **8.4 例子** ###
    1) 等待多个Sockets消息的处理请参见push-pull2的worker


## **9. 处理多部分的消息(Multi-part Messages)** ##
> ### **9.1 在发送端** ###
        zmq_msg_send(socket, &message, ZMQ_SNDMORE);
        ...
        zmq_msg_send(socket, &message, ZMQ_SNDMORE); 
        ...
        zmq_msg_send(socket, &message, 0);
> ### **9.2 在接收端(接收一个完整的消息)** ###
        while (1) {
          zmq_msg_t message;
          zmq_msg_init(&message);
          zmq_msg_recv(socket, &message, 0);
          // process the message frame
          zmq_msg_close(&message);
          int more;
          size_t more_size = sizeof(more);
          zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
          if (!more)
            break;
        }
> ### **9.3 Multi-part需要知道的** ###
    1) 在你发送multi-part消息的时候, 消息的第一部分以及后面的部分事实上只是
       发送到了线路上知道最后一部分发送为止;
    2) 当你使用zmq_poll的时候, 当你接收到第一部分消息, 那么所有剩下的部分都
       到了;
    3) 你将接收消息的所有部分, 否则一部分也接收不到;
    4) 消息的每一部分都是一个zmq_msg_t类型的item;
    5) 你将接收到消息的所有部分无论你是否设置RCVMORE;
    6) 在发送的时候, 0MQ消息队列是存放在内存中的指导最后一个消息进入队列的时
       候, 然后将所有消息发送出去;
    7) 没有办法取消一部分已经发送的消息, 除非关闭socket;


## **10. 扩展的Reque-Reply模式** ##
    见图ex-req-rep.png
    1) 可以让REQ和REP形成N:N的形式
    2) 不必在N:N的情况下创建多个socket
    3) 在REQ和REP的中间创建一个代理将信息分发到REQ和REP
    4) 一种使用zmq_poll(ROUTER-DEALER)的实现请参见ex-req-rep/
    5) 使用0MQ内部的proxy(ROUTER-DEALER)请参见ex-req-rep2/


## **11. 0MQ错误处理** ##
> ### **11.1 在C中的0MQ错误处理** ###
    在真实的代码中, 每一个0MQ调用都要进行出错处理
    1) 创建对象的函数是在则返回NULL
    2) 处理数据的函数返回处理的数据字节数, 返回-1则表示失败或出错
    3) 其他函数返回0表示成功-1表示出错或失败
    4) 错误代码有errno或zmq_errno()提供
    5) 错误描述信息由zmq_strerror()提供
> ### **11.2 不需要错误处理的例外** ###
    1) 当一个线程调用zmq_msg_recv使用了ZMQ_DONTWAIT属性的时候, 将不会等待数
       据的返回; 0MQ将返回-1并将errno设置为EAGAIN
    2) 当一个线程调用了zmq_ctx_destroy而其他线程在做阻塞操作的时候, 这个线程
       将关闭Context, 并且其他所有的阻塞调用将返回-1退出, 并将errno设置成
       ETERM


## **12. 处理中断信号** ##
        当程序接收到Ctrl-C或其他中断信号(SIGTERM)的时候应该被干净的关闭掉, 
    如果简单的kill程序的话, 消息不会被flushed, 文件不会干净的关闭掉。
> ### **12.1 处理中断的例子** ###
    1) 请参见interrupt/
> ### **12.2 0MQ中断的反应** ###
    1) 如果代码在zmq_msg_recv, zmq_poll, zmq_msg_send阻塞, 当一个信号来的时
       候, 这些函数会返回EINTR
    2) 应该包装类似zmq_recv的函数, 当中断的时候让他们返回NULL



## **13. OMQ多线程** ##
        创建完全的完美多线程程序, 我们不需要mutexes, locks, 或其他形式的线程
    间通信除了0MQ sockets交互而发送的消息
> ### **13.1 使用0MQ创建多线程需要遵守的** ###
    1) 禁止在多线程中访问同样的数据; 在0MQ程序中使用传统的MT技术(类mutex)是
       反模式的; 唯一的例外是0MQ context对象, 它是线程安全的
    2) 你必须为你的进程创建一个0MQ Context, 并且要把它传递到所有你希望使用
       inproc sockets创建连接的线程
    3) 你应该向独立进程一样信任线程, 他们有自己的context, 但是这写线程不能通
       inproc来通信; 然而他们却很容易分解成单独的进程
    4) 禁止在线程之间共享0MQ sockets, 0MQ sockets不是线程安全的; 从技术上来
       说是可以实现的, 但是需要使用semaphores, locks或者mutexes; 这会让你的
       程序变慢便脆弱; 唯一的可以在线程间共享sockets的地方是语言绑定的类如垃
       圾回收;
    5) 不要在线程中使用或关闭sockets, 除非他们是在线程创建的;
> ### **13.2 多线程REP服务器** ###
    例子请参见mt-req-rep/
    示例图请参见mt-req-rep.png
    1) 服务器启动了一系列的工作线程, 每一个线程创建一个REP socket, 然后处理
       这个socket上的请求, 就像一个单线程的服务器一样; 唯一的区别是传输通道
       (inproc替换了tco)和绑定-连接的方向
    2) 服务器创建一个ROUTER socket来和客户通信, 将其绑定到外部接口(tcp);
    3) 服务器创建一个DEALER socket来和工作者通信, 将其绑定到内部接口(inproc)
    4) 服务器启动一个proxy来连接两个sockets, 这个proxy公平的接收所有客户请求
       并把这些信息分发到内部的工作者上, 并且可以将路由回复到请求信息的起点;
> ### **13.3 线程间的信号(通信)(PAIR Sockets)** ###
        当你创建多线程程序的时候, 可能会考虑如何在线程之间协调, 尽管你可能会
    使用`sleep`, 或者使用信号量, 互斥等; 但是在0MQ中唯一的机制就是你应该使用
    0MQ消息;
        请见例子mt-ready/   => 讲述了多线程之间执行每一步的顺序的协调
        示意图请见mt-ready.png
        0MQ的该多线程模式解释如下:
        1) 两个线程之间通过inproc来通信, 使用一个共享的Context
        2) 父线程创建一个socket绑定到inproc://endpoint, 然后启动一个子线程, 
           并将Context传递进去
        3) 子线程创建一个socket连接到inproc://endpoint, 然后发信号到父线程表
           明它已经准备好了
        4) PAIR是多线程间通信最好的模式


## **14. 节点协作** ##
        当你需要节点之间的通信的时候, PAIR模式将不再合适, 这就是针对线程和节
    点之间的策略的不同, 大部分的节点来来回回而线程是静态的; PAIR socket不能
    自动重连上离开又重新回来的远端节点;
        节点和线程之间的另外一个不同是, 你通常有一个固定数量的线程, 而节点的
    连接数量确是动态的;
        
        示意图请参见sync-pub-sub.png
> ### **14.1 列子** ###
    早先的PUB-SUB例子的改进版, 其工作模式如下:
    1) pub应该事先知道有多少sub需要连接, 这个值可以从其他地方获得;
    2) pub启动并等待所有的sub连接上, 这就是节点协作的一部分, 每一个sub订阅后
       再通知pub可以等待其他sub的连接了;
    3) 当所有的sub连接上后, pub开始发布信息;


## **15. Pub-Sub消息信箱** ##
    简单的说就像现实生活中的信箱一样, 在发送出去数据的时候加上发送的地址信息
    标示该信息的信箱...
    示意图请参见pub-sub-envelope.png
    例子请参见pub-sub-envelope/
    当然你在发送信息的时候还可以添加上发送者的地址信息, 请参见该模式示意图:
    pub-sub-envelope-sender.png


## **16. 消息丢失问题处理** ##
    在使用0MQ的各个模型发送消息的过程中出现消息丢失的现象, 其解决方案请参见
    图missing-msg-problem-solver.png, 这是在没有启用命名机制的情况下出现的问
    题;
    在启用命名机制后, 消息如下req-min-envelope.png, 在代码中需要先设置连接的
    名字——zmq_setsockopt(socket, ZMQ_IDENTITY, "Lucy", 4)
> ### **16.1 命名机制注意事项** ###
    1) 必须在连接之前设定连接的名字
    2) 不能够重名
    3) 在连接之后不能再修改名字
    4) 不要随机命名
    5) 如需要获知消息来源的名字, 需要在消息发送时附加上(xrep会自动获取)名字


## **17. 各种连接组合** ##
> ### **17.1 Request-Reply Combinations** ###
        我们有4个请求-应答的sockets, 每一个都尤其独特的特性, 我们已经看到他
    们怎么使用简单和扩展的request-reply模式连接, 但是那些socket都是固定建立
    的, 你可以使用他们来解决很多问题
        合法的组合:
        1) REQ to REP
        2) DEALER to REP
        3) REQ to ROUTER
        4) DEALER to ROUTER
        5) DEALER to DEALER
        6) ROUTER to ROUTER
> ### **17.2 REQ to REP组合** ###
        REQ的客户必须启动消息流, REP的服务不能在REQ客户还没发送第一个请求之
    前和它通信; 从技术上来说也是不合法的, API会返回EFSM错误 
> ### **17.3 DEALER to REP组合** ###
        现在将REQ的客户换成DEALER, 它给我们提供一个异步客户可以同多个REP服务
    通信; 如果我们使用DEALER来重写客户端的话, 我们可以发送任意多的请求而不需
    要等待应答;
        当我们使用DEALER和REP socket通信的时候, 我们必须准确的仿真信封以保证
    REQ socket已经发送了, 否则REP socket会将信息认为是非法的, 所以在发送信息
    的使用, 我们应该:
        1) 发送一个使用了MORE选项设置的空消息帧, 
        2) 发送消息体
        而我们在接受消息的时候:
        1) 接收第一个帧, 如果非空, 忽略整个消息, 
        2) 接收下一个帧的数据将他们传递给程序 。。。
> ### **17.4 REQ to ROUTER组合** ###
        就像我们将REQ换成DEALER一样, 我们可以将REP替换成ROUTER; 这个组合给我
    们提供了一个异步服务器可以同时和多个REQ客户通信; 如果我们用ROUTER来重写
    服务端的话, 可以平行的处理一系列的客户请求;
        我们可以将ROUTER用在两个明显的场景:
        1) 将它用作一个前端socket和后端socket之间消息传递的代理;
        2) 用作一个程序来读取消息并作用于它
        在第一个场景下, 这个ROUTER简单的读取所有的数据帧信息包括打造的身份帧
    信息, 只是盲目的传递这些信息; 而在第二个场景, ROUTER必须知道它发送的应答
    消息封装的格式, 其通信的另一端是REQ socket, ROUTER获取身份帧, 空数据帧, 
    然后是具体数据帧
