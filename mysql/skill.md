# **Skills**
***


## **Summary**
 * 总结整理一些使用MySQL的经验和技巧




## **解决MySQL自动断开**
> ### **问题描述**
    当使用MySQL的时候, 如果同服务器在指定的时间内没有数据通信的时候服务器就
    会主动断开连接(MySQL服务器默认是8小时)
> ### **解决方法**
    在C++中, mysql_real_connect之后设置MYSQL_OPT_RECONNECT属性, 然后在读写的
    时候使用mysql_ping, 如果MySQL服务器断开则会重新连接...
    char reconnect_arg = 1;
    if (0 != mysql_options(m, MYSQL_OPT_RECONNECT, (char*)&reconnect_arg)) {
      //! error ...
    }
    ...
    ...
    //! before read/write
    if (0 != mysql_ping(m)) {
      //! error ...
    }
    //! read/write
    ...
    ...
    
