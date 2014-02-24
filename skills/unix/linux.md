# **Linux Study**
***



## **Summary**
 * 记录一些使用Linux的技巧或命令



## **自学Linux命令**
> ### **每日提示**
    在/home/.bashrc中增加如下一行内容:
    echo "Did you know that:"; whatis $(ls /bin | shuf -n 1)
    这样每次打开终端的时候就会显示每日提醒;
    也可以使用cowsay来实现提醒
    $ sudo apt-get install cowsay
    然后在/home/.bashrc中添加
    cowsay -f $(ls /usr/share/cowsay/cows | shuf -n 1 | cut -d. -f1) $(whatis $(ls /bin) 2>/dev/null | shuf -n 1)
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


## **tmux**
    $ tmux new -s [session name] [cmd]
    $ tmux ls                       => list all sessions
    $ tmux attach -t [session name] => resumes a detached tmux session 
    $ C-b "   => split the current pane into two, top and bottom
    $ C-b %   => split the current pane into two, left and right
    $ C-b &   => kill the current window
    $ C-b c   => create a new window
    $ C-b 0~9 => select windows 0 to 9
    $ C-b d   => detach the current tmux client 
    $ C-b o   => select the next pane in the current window 
    $ C-b x   => kill the current pane
    $ C-b n   => change to the next window 
    $ C-b p   => change to the previous window 



## **screen**
    $ screen -S [session name]
    $ screen -ls              => print a list of screen sessions
    $ screen -r [screen_pid]  => resumes a detached screen session
    $ C-a c   => create a new window
    $ C-a w   => show a list of window 
    $ C-a n   => switch to the next window
    $ C-a p   => switch to the previous window
    $ C-a d   => detach screen from this terminal



## **gdb调试core**
    Linux系统在默认的状况下, 程序崩溃是不会产生core文件的, so我们需要打开设
    置, 让程序崩溃的时候产生core文件以便于调试
    $ ulimit -c unlimited
    $ gdb xxx core.xxx
    为了在gdb调试中尽快定位到程序崩溃了的地方, 我们需要在gdb调试中使用
    $ where/bt



## **清除Linux运行后的记录**
    $ history -c
    $ rm ~/.bash_history 
    或者是重新定义exit命令, 在.bashrc文件中添加
    alias exit='history -c; exit'
    那么在每次退出之后就会自动清除当次登录运行的命令



## **Linux上github的push 403错误**
    $ vim .git/config
    将url = https://github.com/<user_name>/<proj_name>.git修改为
      url = https://<user_name>@github.com/<user_name>/<proj_name>.git
