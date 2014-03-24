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
 


## **MyISAM和InnoDB的选择**
    最主要的区别InnoDB支持事务处理与外键和行级锁, MyISAM不支持;
    从稳定性和扩展性以及高可用性来说, MyISAM是首选;
    1. 对于读多写少的需求, MyISAM的读性能比InnoDB好很多;
    2. MyISAM的索引和数据是分开的, 并且索引是有压缩的, 内存使用率高很多, 能
       加载更多的索引;
       InnoDB是索引和数据捆绑在一起的, 没有压缩而从使得InnoDB比MyISAM体积大
       很多;
    4. 对于select count(*)和order by, 如果where不是主键的情况下也是锁全表的;
    4. 基于索引的update, InnoDB的性能更高;
    5. 大批量的inserts语句MyISAM会快一些, 但是updates在InnoDB会更快(尤其是并
       发量大的时候);



## **设置定时任务**
    1. 查看event是否开启
       mysql> show variables like '%sche%';
    2. 开启event_scheduler
       mysql> set global event_scheduler = 1;
    3. 创建事件
       create event if not exists `ev_demo` 
       on schedule every 1 day starts '2014-01-01 00:00:00' 
       on completion preserve do call p_demo();
    4. 关闭事件任务
       alter event `ev_demo` on completion preserve disable;
    5. 开启事件任务
       alter event `ev_demo` on completion preserve enable;
