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
    如果需要让调生成的试信息对gdb更有帮助, 应该使用-ggdb编译选项;
    如果有命令行参数, 可以进入gdb之后使用set args来设置;
    (gdb) set args 111 222 333 444
    (gdb) r 
    (gdb) where/bt



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


## **在CentOS上安装高版本的GCC**
    $ cd /etc/yum.repos.d/ 
    $ wget http://people.centos.org/tru/devtools/devtools.repo 
    $ yum --enablerepo=testing-devtools-6 install devtoolset-1.0
    在用户目录下的.bashrc文件中添加 
    export PATH=/opt/centos/devtoolset-1.0/root/usr/bin/:$PATH



## **CentOS入侵后检查**
    1) 检查用户登录记录
        $ more /var/log/secure 
        $ who /var/log/wtmp 
        $ more /var/log/messages 
    2) 在root帐号下输入history, 看最近的1000条记录
    3) 检查/etc/inetd/conf 
        $ cat /etc/inetd.conf | grep -v "^#"
        得到的信息就是这台机器所开启的远程服务
    4) 检查网络连接和监听端口
        $ netstat -an   => 列出本机所有连接和监听端口, 查看有无非法连接
        $ netstat -rn   => 查看本机的路由, 王冠设置是否正确
        $ ifconfig -a   => 查看网卡设置
    5) 检查系统日志
        $ last | more 查看在正常情况下登录到本机的所有用户历史记录;
        入侵者通常会停止系统syslog, 查看系统syslog进程的情况, 判断syslog上次
        启动的时间是否正常, syslog是以root身份执行的, 如果syslog被非法动过,
        则说明有重大入侵事件;
        
        $ ls -la /var/log 
        检查wtm, utmp, messages文件的完整性和修改时间是否正常;
    6) 重要数据备份
        非Linux上的原有数据, /etc/passwd, /etc/shadow, WWW网页数据, /home里
        的用户文件备份
    7) 重装系统



## **CentOS入侵后处理**
    1) 修改密码/限制ssh登录
        $ passwd 

        限制ssh IP登录
        $ vim /etc/hosts.allow 
        sshd:192.168.10.88:allow或者sshd:192.168.10.0/24:allow 
        $ vim /etc/hosts.deny
        sshd:ALL 
    2) 查看登录情况
        $ more /var/log/secure 
        $ last -f /var/log/wtmp 
    3)杀死陌生进程(ps -ef | grep xxx, kill)
    4) 查看网络情况
        安装iptraf
        对陌生端口进行查看 $ lsof -i:端口号, 查看占用这个端口的进程
        查询进程, 杀死进程


## **Terminal常用技巧**
> ### **1. 光标移动**
    * Ctrl+a    移动到行首
    * Ctrl+e    移动到行尾
    * Alt+f     向前移动一个单词
    * Alt+b     向后移动一个单词
    * Ctrl+f    向前移动一个字符
    * Ctrl+b    向后移动一个字符
    * Ctrl+xx   在光标当前位置和行首之间切换
> ### **2. 命令编辑**
    * Ctrl+d    删除光标所在位置的字符
    * Ctrl+h    删除光标前的一个字符，效果同Backspace
    * Ctrl+w    删除光标所在位置的单词(向前)
    * Alt+d     删除光标所在位置的单词(向后)
    * Ctrl+l    清空当前屏幕
    * Ctrl+k    剪切光标位置到行尾的内容到剪切板
    * Ctrl+u    剪切光标位置到行首的内容到剪切板
    * Ctrl+y    将剪切板的内容复制出来
> ### **3. 命令历史**
    * Ctrl+r    搜索最近输入的命令
    * Ctrl+p    上一个命令
    * Ctrl+n    下一个命令
    * !!        重复执行上一个命令
> ### **4. Bash中的emacs模式和vi模式**
    1) 上述命令是emacs模式的，默认是emacs模式
    2) 设置终端的vi模式(设置后可以使用vi命令来操作终端):
        $ set -o vi
    3) 设置回emacs模式:
        $ set -o emacs
