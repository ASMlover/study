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
