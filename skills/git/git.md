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





## **2. 在linux上搭建基于ssh的git服务器**
> ### **2.1 服务端操作**
    $ cd hkr/code
    $ mkdir gitproj
    $ cd gitproj
    $ git --bare init
> ### **2.2 客户端操作**
    $ git clone ssh://hkr@192.168.125.100:22/home/hkr/code/gitproj
    输入Linux上hkr的帐号密码
    提交流程如下:
    $ git add .
    $ git commit
    $ git push origin master



## **3. Git命令**
    1) 检查出谁动了我的代码:
       会显示文件中每一行的作者, 最后一次改动进行的提交以及该次提交的时间;
       $ git blame [file_name]
    2) 回顾仓库历史(git log)
       * --oneline 把每次提交显示的信息压缩成hash值和提交信息在一行显示;
       * --graph 会在输出界面的左手边用一种基于文本的图形表示法来显示历史;
       * --all 显示全部分支的历史;
    3) 绝不丢失一个提交信息(git reflog)
       git reflog显示的是所有head移动的信息; 它是在本地的, 不是仓库的一部分,
       不会包含在推送和合并中;
       git reflog显示一个提交信息(b1b0ee9 - HEAD@{4}), 这是使用(hard reset)
       时丢失的那个;
    4) 合并多次提交
       把多次提交合并为一个:
       $ git rebase -i HEAD~[number_of_commits]
       $ git rebase -i HEAD~2 => 合并最后两次提交
    5) 保存尚未提交的修改(git stash)
       stash的本质是保存你全部的改动以供将来使用:
        $ git stash
        * 查看暂存列表: $ git stash list
        * 不想保存或恢复这些改动: $ git stash apply
        * 只使用某些保存, 需要在apply命令后加上指明标示符:
          $ git stash apply stash@{2}
    6) 检查丢失的提交
       应对大型仓库的时候reflog就不行了:
       $ git fsck --lost-found
       在看到丢失的提交后, 可以使用git show [commit_hash]来查看这些提交所包
       含的改动或使用git merge [commit_hash]来恢复提交;
       fsck在你删除一个远端分支并克隆了仓库的时候, 也可以搜索并恢复该远端的
       分支;
    7) cherry-pick命令
       从不同的分支里选择某次提交并把它合并到当前的分支来;
       一个BUG, 如果你在一个分支中修改了它, 可以使用cherry-pick把这次提交合
       并到其他的分支而不会搞乱其他的文件或提交;
        * 首先得到某个提交的 commit_hash
        * 切换到想要使用该提交的分支
        * $ git cherry-pick [commit_hash]

## **4. github只clone一个repository的subdirectory**
``` bash
    $ mkdir <repo>
    $ cd <repo>
    $ git init
    $ git config core.sparsecheckout true
    $ echo "<some>/<dir>" >> .git/info/sparse-checkout
    $ echo "<another>/<sub>/<dir>" >> .git/info/sparse-checkout
    $ git remote add -f origin <repo-url>
    $ git pull origin master
```
    推送的时候请使用`git push origin master`,  拉取的时候使用`git pull origin master`, 例子如下：
``` bash
    $ mkdir study
    $ cd study
    $ git init
    $ git config core.sparsecheckout true
    $ echo "cplusplus/tyr2" >> .git/info/sparse-checkout
    $ git remote add -f origin https://github.com/ASMlover/study.git
    $ git pull origin master
```

## **5. git使用submodule(子模块)**
``` bash
    $ cd <repo>
    $ git submodule add <submodule-repo-git-address> <target-dir>
```
    例子如下：
``` bash
    $ cd Chaos
    $ git submodule add https://github.com/ASMlover/Surtr.git Tools/Surtr
```

    删除子模块，直接使用一条命令:
    $ git rm <submodule_name>
