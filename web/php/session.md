# **会话控制** #
*** 


## **1. 为什么要使用会话控制** ##
        会话控制的思想就是允许服务器跟踪同一个客户端做出的连续请求, 这样, 我
    们就可以很容易的做到用户登陆的支持, 而非在每浏览一个网页都去重复执行登陆
    的动作。




## **2. 会话跟踪的方式** ##
        HTTP是无状态协议, 不能维护两个事务之间的状态, 但一个用户在请求一个页
    面以后再请求另一个页面, 还要让服务器知道这是同一个用户, PHP提供如下方式:
    1) 使用超链接或header()函数等重定向方式, 在URL中附加参数的形式将数据从一
       个页面转向另一个PHP脚本中; 
       也可以通过网页中各种隐藏表单来存储信息并将这些信息在提交表单时传递给
       服务器中的PHP脚本;
    2) 使用Cookie将用户的状态信息, 存放到客户端的电脑中, 让其他程序通过存取
       客户端电脑的Cookie来存取目前的使用者资料信息;
    3) 使用Session, 将访问者的状态信息存放在服务器上, 让其他程序能透过服务器
       的文件或数据库, 来存取使用者资讯;




## **3. Cookie的应用** ##
        Cookies是一种由服务器发送给客户端的片段信息, 存储在客户端的浏览器的
    内存或硬盘上, 在客户对该服务的请求中发回它
        (Cookie就好比会员卡, 由商场提供, 你购物的时候需要你提供)
> ### **3.1 向客户端的电脑中设置Cookie** ###
    1) 需要用户的浏览器支持Cookie功能
    2) 使用PHP内建的setcookie来建立一个Cookie
    3) setcookie必须在其他信息被输出到浏览器之前调用
    4) bool setcookie(string $name[, string $value[, int $expire
                    [, string path[, string $domain[, bool secure]]]]]);
        $name => Cookie的识别名称 
        $value => Cookie的值, 可为数值或字符串形态, 保存在客户端, 非敏感数据
        $expire => Cookie生存期限, UNIX时间戳
        $path => Cookie在服务端指定的路径
        $domain => 指定该Cookie所属服务器的网址名称
        $secure => 指明Cookie是否仅通过安全的HTTPS连接传送
> ### **3.2 在PHP脚本中读取Cookie的资料信息** ###
    通过$_COOKIE来访问相应的Cookie信息
> ### **3.3 删除Cookie** ###
    1) 省略setcookie函数的所有参数列表, 仅导入第一个参数Cookie识别名称参数
    2) 利用setcookie, 把目标Cookie设定"已过期"状态




## **4. Session的应用** ##
        在客户端仅需要保存由服务器为用户创建的一个Session标识符, 称为Session
    ID; 而在服务端保存Session变量的值;
        Session是存放在服务器的, 为了避免对服务器造成过大的负荷, 因此Session
    会因为以下两种状况而自然消失;
        1) 当使用者关闭浏览器, 失去与服务器的连接后, Session自动消失;
        2) Session指定的有效期限到期; 
            可以在php.ini中的session.cookie.life_time来设置, 单位为秒;
            默认0表示, 直到关闭浏览器为止;
> ### **4.1 配置Session** ###
        配置信息在php.ini中
    1) session.auto_start
        自动启动会话, 0表示禁用, 1表示开启
    2) session.cache_expire
        为缓存中的会话页设置当前时间, 单位是分钟
    3) session.cookie_domain
        指定会话Cookie中区域
    4) session.cookie_lifetime
        Cookie中的Session ID在客户机上保存的时间, 0表示直到浏览器关闭
    5) session.cookie_path
        在会话Cookie中要设置的路径
    6) session.name
        会话的名称, 在客户端用作Cookie的标识名称
    7) session.save_path
        会话咋服务端存储的路径
    9) session.use_cookies
        配置在客户端使用Cookie的会话, 1表示允许
> ### **4.2 Session的声明与使用** ###
    1) Session必须先启动
    2) session_start
        bool session_start(void); => 创建Session, 开始一个会话
        开始一个会话, 返回已经存在的会话
    3) 在开启Session之前不能有任何输出的内容
    4) 在php.ini中设置session.auto_start=1就不必每个脚本都调用session_start
    5) 类定义必须在Sessoion之前加载, 所以不建议使用php.ini的属性开启Session
> ### **4.3 注册一个会话变量和读取Session** ###
    1) 注册, 读取Session变量都要通过访问$_SESSION完成
    2) 必须在调用session_start开启Session之后才能使用
> ### **4.4 注销变量与销毁Session** ###
    1) 销毁Session
        使用函数session_destroy销毁和当前Session有关的所有资料
        bool session_destroy(void);
    2) session_destroy不一定成功, 如果失败是不会释放和当前Session相关的变量,
       也不会删除保存在客户Cookie中的Session ID; 
       需要使用unset来释放在Session中注册的单个变量; 但是*绝对*不能使用unset
       来删除整个$_SESSION数组;
    3) 如果想删除Session中的所有变量, 直接将$_SESSION赋值一个空数组
    4) Session ID是保存在客户端的Cookie中的, 所以需要调用session_name来获取
       Session的名称, 再使用setcookie来删除客户端Cookie的Session ID
> ### **4.5 传递Session ID** ###
    1) 通过Cookie传递Session ID
        如果客户端没有禁用Cookie, 则在PHP脚本中通过session_start进行初始化后
        服务器会自动发送HTTP标头将Session ID保存草客户端的Cookie中, 类似:
          setcookie(session_name(), session_id(), 0, '/');
    2) 通过URL传递Session ID 
        如果客户端浏览器不支持Cookie, PHP可以重写客户请求的URL, 把Session ID
        添加到URL信息中, 可以手动在每个超链接的URL中添加一个Session ID。
        可以使用SID常量, 该常量在会话启动时被定义, 如果客户端没有发送适当的
        会话Cookie, 则SID的格式为session_name=session_id, 因此可以无条件的将
        其嵌入到URL中;
