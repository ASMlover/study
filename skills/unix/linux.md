# **Linux Study**
***



## **Summary**
 * 记录一些使用Linux的技巧或命令



## **自学Linux命令**
> ### **每日提示**
    在/home/.bashrc中增加如下一行内容:
    echo "Did you know that:"; whatis $(ls /bin | shuf -n 1)
    这样每次打开终端的时候就会显示每日提醒
> ### **使用whatis命令**
    whatis会逐条解释你输入的命令
> ### **浏览命令的所有可用选项**
    1. man <command>
    2. <command> --help
> ### **速成**
    所有Linux命令都具有相同的结构:
    [sudo] program [param][param]...[param][-flag][param]...[-flag][param]
    1. 如果一个命令需要管理员权限, 就必须在它之前加上"sudo"
    2. "program"是应用程序的名字; 如没有额外配置放在/usr/bin下面
    3. 在程序名字后面, 可以使用不同的参数和标志



## **查看Linux版本/系统信息**
    $ lsb_release -a    : 查看发行版信息
    $ uname             : 查看系统信息
    $ uname -a
    $ getconf LONG_BIT  : 看32位 | 64位
    $ echo $HOSTTYPE    : 查看CPU架构
