# **epoll** #
***


## **1. 简单的epoll例子** ##
    1) 接口
        * int epoll_create(size);
          建立epoll并返回其描述符
        * int epoll_ctl(int epfd, int op, int fd, struct epoll_event* ev);
          将socket描述符(一般情况是), 交给epoll并设定触发条件
        * int epoll_wait(int epfd, struct epoll_event* events, 
                         int maxevents, int timeout);
          等待已经注册的事件被触发或终止
    2) epoll提狗edge-triggered和level-triggered模式, ET模式中, epoll_wait仅
       会在新的事件首次被加入epoll时返回; LT模式中, epoll_wait在事件状态未变
       更之前将不会触发
    3) 具体例子请参见./demo00/
