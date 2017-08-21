# **README for network** #
***


## **What's network** ##
  * Records knowledge about network both about Windows/Linux.
  * Just for studying and for fun.
  * `CMakeLists.standalone`是单独编译asio的cmake脚本

## **UDP Programming**
  * UDP是一种**非连接的、不可靠的数据报文协议**
  * UDP客户端不需要和服务端建立连接，可以直接使用sendto来发送数据报文
  * UDP服务端不需要accept客户端的连接，直接使用recvfrom来接收客户端发送的数据报文
  * UDP客户端可以使用read、write替换recvfrom和sendto
  * UDP服务端必须使用recvfrom和sendto，因为UDP没有连接，必须指定struct sockaddr
  * UDP服务器特点
    - 服务器不接受客户端连接，只需要监听端口
    - 循环服务器，可交替处理各客户端数据报，不会被一个客户端独占
  * UDP客户端connect
    - 与TCP connect不同，没有三次握手，内核只检查是否存在立即可知的错误，记录对端的IP和端口，理解返回到调用进程
    - 无连接需要使用sendto和recvfrom
    - 已连接需要使用send和recv，不能使用sendto
    - connect在UDP中用来检查UDP端口是否开放、没被使用
    - 一旦UDP调用了connect则不再使用recvfrom来获取数据报的发送者，而直接使用read、recv，内核会自动填写IP地址与端口给已connect的socket数据报
    - 在wuconnect的情况下，使用非指定地址的函数（write发送数据），将失败
    - 未连接的UDP调用sendto，内核会暂时连接该socket，然后发送数据报载断开；因此，当应用程序知道要给同一个socket发哦是那个多个数据报时，显示连接的UDP效率更高
```c++
  #include <sys/socket.h>

  // 每次调用sendto都必须指明接收方的socket地址，UDP协议没有设置发送缓冲区，sendto将数据拷贝
  // 到系统缓存后返回，通常不会阻塞，允许发送空数据报（返回0）
  int sendto(int sockfd, const void* buffer, int length, unsigned int flags, const struct sockaddr* to, int tolen);

  // UDP为每个UDP socket设置了一个接收缓冲区，每个收到的数据报根据其端口放在不同的缓冲区；
  // recvfrom每次从缓冲区取回一个数据报，没数据的时候会阻塞，返回0标识接收到一个长度为0的数据报，
  // 并不是表示对方已经结束发送
  int recvfrom(int sockfd, void* buffer, int length, unsigned int flags, struct sockaddr* from, int* fromlen);
```
