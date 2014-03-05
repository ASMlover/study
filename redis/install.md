# **Install for redis**
***


## **1. 服务器安装**
    $ wget http://download.redis.io/releases/redis-2.8.6.tar.gz
    $ tar xzvf redis-2.8.6.tar.gz
    $ cd redis-2.8.6
    $ make
    然后编译的二进制文件就在src目录下了, 启动redis服务
    $ src/redis-server

    启动redis内置的客户端
    $ src/redis-cli
    redis> hset player:account01 id 1
    OK
    redis> hget player:account01 id 
    "1"



## **2. 使用hiredis**
    hiredis是官方支持的redis C客户端
    $ git clone https://github.com/redis/hiredis.git
    $ cd hiredis
    编译静态库;
    $ make static
    然后就可以使用了;
