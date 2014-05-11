# **IO模型**
***



## **select**
    允许指示内核等待多个事件中的任何一个发生, 并只在有一个或多个事件发生或经
    历一段制定的时间后才唤醒它;
    返回值表示所有描述符的已就绪的总数, 在任何描述符就绪之前定时器到时返回0,
    返回-1表示出错;

    有数据可读 => 可读
    关闭连接的读一半 => 可读
    给监听套接口准备好新连接 => 可读
    有可用于写的空间 => 可写
    关闭连接的写一半 => 可写
    待处理错误 => 可读可写
    TCP带外数据 => 异常



## **poll**
    POLLIN      可为events输入, 可为revents结果, 普通或优先级带数据可读
    POLLRDNORM  可为events输入, 可为revents结果, 普通数据可读
    POLLRDBAND  可为events输入, 可为revents结果, 优先级带数据可读
    POLLPRI     可为events输入, 可为revents结果, 高优先级数据可读
    POLLOUT     可为events输入, 可为revents结果, 普通数据可写
    POLLWRNORM  可为events输入, 可为revents结果, 普通数据可写
    POLLWRBAND  可为events输入, 可为revents结果, 优先级带数据可写
    POLLERR                     可为revents结果, 发生错误
    POLLHUP                     可为revents结果, 发生挂起
    POLLNVAL                    可为revents结果, 描述符不是一个打开的文件

    发生错误时, 返回-1; 超时返回0; 否则返回就绪描述符个数;
    如果不再关心某个特定描述符, 可把与它对应的pollfd结构的fd成员设置成一个
    负值, poll函数将忽略这样的pollfd结果的events成员, 返回时将它的revents成
    员的值设置为0;
