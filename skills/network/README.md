# **Network知识信息** #
***


## **1. 判断远端断开的方法** ##
> ### **方法1** ###
    recv返回值小于等于0时, socket连接断开; 还需要判断errno是否等于EINTR, 如
    果errno == EINTR则说明recv函数是由于程序接收到信号后返回的, socket连接
    还是正常的, 不应该close掉socket连接
> ### **方法2** ###
    struct tcp_info info;
    int len = sizeof(info);
    getsockopt(s, IPPROTO_TCP, TCP_INFO, &info, (socklen_t*)&len);
    if (info.tcpi_state == TCP_ESTABLISHED) 说明未断开 else 则断开
> ### **方法3** ###
    使用select等系统函数, 若远端断开, 则select返回1, recv返回0则断开; 其他
    注意事项需要检查errno
> ### **方法4** ###
    int keep_alive = 1; // 开启keep alive属性
    int keep_idle = 60; // 如连接在60秒内没数据往来, 则进行探测
    int keep_interval = 5; // 探测时发包的时间间隔为5秒
    int keep_count = 3; // 探测尝试的次数
    setsockopt(fd, SOL_SOCKET, 
      SO_KEEPALIVE, (void*)&keep_alive, sizeof(keep_alive));
    setsockopt(fd, SOL_TCP, 
      SO_KEEPIDLE, (void*)&keep_idle, sizeof(keep_idle));
    setsockopt(fd, SOL_TCP, 
      TCP_KEEPINTVL, (void*)&keep_interval, sizeof(keep_interval));
    setsockopt(fd, SOL_TCP, 
      TCP_KEEPCNT, (void*)&keep_count, sizeof(keep_count));
    设置后, 若断开, 则在使用该socket读写时立即失败, 并返回ETIMEDOUT 
> ### **方法5** ###
    自己实现一个心跳包



## **Socket API常用头文件**
    sys/types.h             所有需要的基本类型
    sys/socket.h            所有套接字数据结构
    netinet/in.h            IPv4/IPv6所需要的全部函数
    unistd.h                查找本地机器名所需要的gethostname
    netdb.h                 所有需要的DNS函数
    arpa/inet.h             以inet_开头的所有函数
    errno.h                 所有错误处理信息
    fcntl.h                 所有文件控制信息


## **socket错误代码**
    ENETDOWN        网络已经失败并断开
    EAFNOSUPPORT    不支持指定的地址族
    EINPROGRESS     对此函数的调用仍在进行中, 因此不能完成新的调用
    EMFILE          没有更多可以使用的套接字描述符
    ENOBUFS         没有足够的内存可以使用
    EPROTONOSUPPORT 不支持指定的协议
    EPROTOTYPE      套接字类型不支持指定的协议
    ESOCKTNOSUPPORT 地址族不支持指定的套接字类型


## **bind的错误代码**
    ENETDOWN        网络已经失败并断开
    EINPROGRESS     对此函数的调用仍在进行中, 因此不能完成新的调用
    ENOBUFS         没有足够的内存可以使用
    ENOTSOCK        传递过来的套接字描述符不是真正的套接字
    EACCESS         访问被否决
    EADDRINUSE      地址已经在使用中
    EADDRNOTAVAIL   地址对这台机器无效
    EFAULT          一个或多个参数无效


## **listen错误代码**
    ENETDOWN        ~
    EADDRINUSE      ~
    EINPROGRESS     ~
    EINVAL          套接字无效
    EISCONN         套接字已经被连接
    EMFILE          ~
    ENOBUFS         ~
    ENOTSOCK        ~
    EOPNOTSUPP      套接字不支持这一函数


## **accept错误代码**
    ENETDOWN        ~
    EINPROGRESS     ~
    EINVAL          ~
    EMFILE          ~
    ENOBUFS         ~
    ENOTSOCK        ~
    EOPNOTSUPP      ~
    EFAULT          ~
    EWOULDBLOCK     函数由于阻塞而退出


## **connect错误代码**
    ENETDOWN        ~
    EINPROGRESS     ~
    EADDRINUSE      ~
    EINVAL          ~
    EADDRNOTAVAIL   ~
    EAFNOSUPPORT    ~
    ENOBUFS         ~
    ENOTSOCK        ~
    EWOULDBLOCK     ~
    ECONNREFUSED    服务器拒绝连接
    EFAULT          ~
    EISCONN         ~
    ENETUNREACH     目的地址不可达到
    ETIMEDOUT       由于超时, 操作未能完成


## **send错误代码**
    ENETDOWN        ~
    EINPROGRESS     ~
    EACCESS         ~
    EFAULT          ~
    ENETRESET       网络已经复位, 连接已经断开
    ENOBUFS         ~
    ENOTCONN        套接字没有被连接
    ENOTSOCK        ~
    EOPNOTSUPP      ~
    ESHUTDOWN       套接字已经被关闭
    EWOULDBLOCK     ~
    EHOSTUNREACH    主机不可达
    EINVAL          ~
    ECONNABORTED    连接异常中止, 套接字不再可用
    ECONNRESET      连接被对方关闭
    ETIMEDOUT       连接被出乎意料的关闭


## **recv错误代码**
    ENETDOWN        ~
    EFAULT          ~
    ENOTCONN        ~
    EINPROGRESS     ~
    ENETRESET       ~
    ENOTSOCK        ~
    EOPNOTSUPP      ~
    ESHUTDOWN       ~
    EWOULDBLOCK     ~
    EINVAL          ~
    ECONNABORTED    ~
    ETIMEDOUT       ~
    ECONNRESET      ~
