# **Redis持久化**
***


## **1. RDB优点**
  * RDB是一个非常紧凑的文件, 保存了某时间点的数据集, 适用于数据集备份;
  * RDB是一个紧凑的单一文件, 方便传送到另一个远端数据中心, 适用于灾难恢复;
  * RDB在保存RDB文件时父进程fork一个子进程, 由子进程来操作, 父进程不需要再做其他IO操作, 所以RDB持久化性能很高;
  * 与AOF相比, 在恢复大数据集时, RDB更方便;


## **2. RDB缺点**
  * 如果希望在redis意外停止工作时丢失的数据最少, 那RDB不适合你;
  * RDB经常需要fork子进程来保存数据集到硬盘上, 当数据集比较大的时候, fork会耗时, 可能导致redis在一些毫秒级不能响应客户端的请求;




## **3. AOF优点**
  * 适用AOF让你的redis更耐久, 可以使用不同的fsync策略(无fsync, 每秒fsync, 每次写的时候fsync, 默认的每秒fsync); Redis性能很好(fsync由后台线程处理, 主线程处理客户端请求);
  * 只进行追加日志文件, 即使由于某些原因无法完成写入命令, 你也可以使用redis-check-aof工具修复这些问题;
  * redis可以在AOF文件变得过大时, 自动在后台对AOF进行重写; 重写的新AOF文件包含了恢复当前数据集所需的最小命令集合, 整个重写操作绝对是安全的, redis在创建新AOF文件时会继续将命令追加在现有的AOF文件; 新AOF创建完毕, redis就会从旧AOF文件切换到新AOF文件, 并开始对新AOF文件进行追加;
  * AOF文件有序保存了对数据库执行的所有写操作;


## **4. AOF缺点**
  * 对相同的数据集, AOF文件体积通常大于RDB文件
  * 根据fsync策略, AOF速度可能慢于RDB;
