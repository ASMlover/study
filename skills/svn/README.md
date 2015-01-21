# **README for svn**
***

## **1. Windows下搭建svn服务器**
> ### **1.1 建立版本库**
    \> cd E:\workspace
    \> svnadmin create repos
    不能递归创建, workspace需要事先建立好
> ### **1.2 修改配置文件**
    1) 在repo/conf下有authz, passwd和svnserve.conf 3个文件, 修改passwd和
       svnserve.conf这两个配置文件:
    2) 在svnserve.conf中, 让anon-access = read, auth-access = write以及
       password-db = passwd生效
    3) 在passwd文件中添加一对用户密码即可
> ### **1.3 启动svn服务器**
    \> cd E:\workspace
    \> svnserve -d -r ./repos
    1) -d/--daemon: 以守护进程方式运行
    2) -r/-root: 设置svn://localhost, 这里svn://localhost是repos目录
    3) 默认端口是3690, 如果该端口被占用则需要使用--listen-port=xxx来设置
> ### **1.4 导入项目**
    \> cd E:\workspace
    \> svn import -m "proj" svn://localhost/proj
> ### **1.5 checkout项目**
    \> svn checkout svn://localhost/proj proj
