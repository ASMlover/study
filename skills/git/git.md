# **GIT服务器的搭建** #
***

## **1. GIT服务器在Windows上的搭建** ##
        在Windows上搭建Git服务器一般采用的是SSL + putty + msysGit来完成的, 
    不过这里主要介绍的是另外一种我更喜欢的方式——基于Apache服务器搭建的Git服
    务器。
        好处是免除了SSH在Windows上的配置, 同时可以使用HTTP(S)协议来访问版本
    库, 具体的搭建方法如下:
> ### **1.1 安装GIT** ###
        由于官方并没有提供Windows上面的安装程序, 这里我们选择msysGit, 其最新
    [地址](https://code.google.com/p/msysgit/)可以下载到最新的版本。
        进入Git安装目录D:\Tools\Git\libexec\git-core, 运行git-http-backend, 
    这个程序的作用是处理HTTP请求, 如果得到错误请将D:\Tools\Git\bin目录下的
    libiconv-2.dll拷贝到D:\Tools\Git\libexec\git-core目录下即可
> ### **1.2 安装Apache服务器** ###
        到官方网站下载最新版本, 安装即可。
> ### **1.3 配置Apache服务器** ###
        打开Apache下的conf目录的httpd.conf文件, 找到<Direcory />, 修改如下:
        <Direcory />
          Options FollowSymLinks
          AllowOverride None
          Order deny,allow 
          allow from all
        </Direcory>
>
        放开httpd.conf的下面两行前的注释
        LoadModule dav_module modules/mod_dav.so
        LoadModule dav_fs_module modules/mod_dav_fs.so
> 
        在httpd.conf文件的末尾添加如下内容:
        # 指定Git版本库的位置
        SetEnv GIT_PROJECT_ROOT E:/workspace/gits
        # 该目录下的所有版本库都可以通过HTTP(S)方式存取
        SetEnv GIT_HTTP_EXPORT_ALL
        # 令Apache把Git相关URL导向Git的HTTP处理程序
        ScriptAliasMatch \
          "(?x)^/(.*/(HEAD |  info/refs |  objects/(info/[^/]+ |  [0-9a-f]{2}/[0-9a-f]{38} |  pack/pack-[0-9a-f]{40}.(pack|idx)) | git-(upload|receive)-pack))$" \
          "D:/Tools/Git/libexec/git-core/git-http-backend.exe/$1"
>
        <Location />
          AuthType Basic
          AuthName "GIT Repository"
          AuthUserFile "E:/workspace/gits/.passwd"
          Require valid-user
        </Location>
>
> 
        <Location />中, Require valid-user的意思是就是需要用户密码验证
> ### **1.4 添加用户** ###
        我们将向.passwd文件中添加用户和密码, 这样我们在提交版本的时, Apache 
    服务器就可以根据我们制定的用户名和密码判断是否具有权限
        我们可以利用Apache\bin目录下的htpasswd命令来生成密码文件
        htpasswd -cmb .passwd hkr 123 (用户: hkr  密码: 123)
        具体的htpasswd添加用户, 删除用户等请参见htpasswd使用手册
> ### **1.5 注意** ###
    1) 生成的密码文件一定要拷贝到E:/workspace/gits下面 
    2) 每一个xxx.git目录下一定要有一个git-daemon-export-ok的空文件
> ### **1.6 测试** ###
    1) 到E:/workspace/gits目录下:
       E:\workspace\gits> git init --bare hello.git 
    2) 在hello.git目录下添加一个git-daemon-export-ok的空文件
    3) 在E:/workspace目录下:
       E:\workspace> git clone http://localhost:4040/hello.git 
    4) 我们在hello目录中添加文件后提交
       E:\workspace\hello> git push origin master
> ### **1.7 相关DEMO文件** ###
    相关的httpd.conf例子文件请参见./win-git-demo/httpd.conf
