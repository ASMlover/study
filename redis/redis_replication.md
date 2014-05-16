# **redis复制**
***


## **Replication**
    redis通过配置允许slave redis servers或者master server的复制品;
    * 一个master可以有多个slave
    * slave能与其他slave连接, 除了可以接收同一个master下面的slave的连接外, 
      还可以接收同一个结构图中的其他slave连接;
    * redis复制在master段是非阻塞的;
    * 复制在slave端也是非阻塞的;
    * 为了有多个slaves可以做只读查询, 复制可以重复多次, 具有可扩展性;
    * 可以利用复制去避免在master端保存数据, 对master端的redis.conf进行配置,
      就可以避免保存, 然后通过slave的连接, 来实时的保存在slave端;
