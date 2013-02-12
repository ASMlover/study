# **数据库抽象层PDO** #
*** 


## **1. PDO所支持的数据库** ##
    1) PDO_DBLIB : FreeTDS/Microsoft SQL Server/Sybase
    2) PDO_FIREBIRD : Firebird/Interbase 6
    3) PDO_MYSQL : MySQL 3.x/4.x/5.x
    4) PDO_OCI : Oracle
    5) PDO_ODBC : ODBC v3
    6) PDO_PGSQL : PostgreSQL
    7) PDO_SQLITE : SQLite 2.x/3.x




## **2. 创建PDO对象** ##
    1) PDO的构造方法
        __construct(string dsn[, string username[, string passwd[, 
                    array driver_options]]]);
    2) dsn格式
        oci:dbname=//localhost:1521/testdb
        mysql:host=localhost;dbname=testdb 
> ### **2.1 PDO与连接有关的选项** ###
    1) PDO::ATTR_AUTOCOMMIT => 确定PDO是否关闭自动提交功能
    2) PDO::ATTR_CASE => 强制PDO获取的表字段字符的大小写转换, 或原样用列信息
    3) PDO::ATTR_ERRMODE => 设置错误处理模式
    4) PDO::ATTR_PERSISTENT => 确定连接是否为持久连接
    5) PDO::ATTR_ORACLE_NULLS => 将返回的空字符串转换为SQL的NULL
    6) PDO::ATTR_PREFETCH => 设置应用程序提前获取的数据大小, 单位为K字节
    7) PDO::ATTR_TIMEOUT => 设置超时之前等待的时间(秒)
    8) PDO::ATTR_SERVER_INFO => 包含与数据库特有的服务器信息
    9) PDO::ATTR_SERVER_VERSION => 包含与数据库服务器版本号有关的信息
    10) PDO::ATTR_CLIENT_VERSION => 包含与数据库客户端版本好有关信息
    11) PDO::ATTR_CONNECTION_STATUS => 包含数据库特有的与连接状态有关的信息
> ### **2.2 PDO对象中的成员方法** ###
    1) getAttribute => 获取一个数据库连接对象的属性
    2) setAttribute => 为一个数据库连接对象设置属性
    3) errorCode => 获取错误码
    4) errorInfo => 获取错误的信息
    5) exec => 处理一条SQL语句, 返回所影响的条目数
    6) query => 处理一条SQL语句, 返回一个"PDOStatement"对象
    7) quote => 为某个SQL中的字符串田间引号
    8) lastInsertId => 获取插入到表中的最后一条数据的主键值
    9) prepare => 准备要执行SQL语句
    10) getAvailableDrivers => 获取有效的PDO驱动器名称
    11) beginTransaction => 开始一个事务, 标明回滚起始点
    12) commit => 提交一个事务并执行SQL
    13) rollback => 回滚一个事务
> ### **2.3 PDOStatement成员方法** ###
    1) bindColumn => 匹配列名和一指定的变量名
    2) bindParam => 将参数绑定到相应的查询占位符上
    3) bindValue => 将一值绑定到相应的一个参数上
    4) closeCursor => 关闭游标, 使该声明再次被执行
    5) columnCount => 在结果集中返回列的数目
    6) errorCode => 获取错误码
    7) errorInfo => 获取错误的信息
    8) execute => 负责执行一个准备好的预处理查询
    9) fetch => 返回结果集的下一行, 当结果到末尾返回FALSE
    10) fetchAll => 获取结果集的所有行
    11) fetchColumn => 返回结果集下一行某个列的值
    12) fetchObject => 获取下一行记录并返回它作为一个对象
    13) getAttribute => 获取一个声明属性
    14) getColumnMeta => 在结果集中返回某一列的属性信息
    15) nextRowset => 检索下一行集
    16) rowCount => 返回使用query执行的SELECT语句后受影响的行数
    17) setAttribute => 为一个预处理语句设置属性
    18) setFetchMode => 设置需要结果集合的类型
