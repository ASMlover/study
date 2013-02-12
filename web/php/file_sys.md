# **文件系统处理** #
*** 


## **1. 文件类型** ##
    在Windows中只有"file", "dir"和"unknown"
    block => 块设备文件
    char => 字符设备是指在IO传输过程中以字符为单位进行传输的设备
    dir => 目录类型, 目录也是文件的一种
    fifo => 命名管道, 用于将信息从一个进程传递到另一个进程
    file => 普通文件类型, 文本文件或可执行文件
    link => 符号链接, 是指向文件指针的指针
    unknown => 未知类型



## **2. 文件的属性** ##
    1) file_exists() => 检查文件或目录是否存在
        参数是文件名, 文件存在返回TRUE, 否则返回FALSE
    2) filesize() => 取得文件大小
        参数是文件名, 返回文件大小的字节数, 出错返回FALSE
    3) is_readbale() => 判断给定文件名是否可读
        参数是文件名, 如文件存在且可读返回TRUE
    4) is_writable() => 判断给定文件名是否可写
        参数是文件名, 如文件存在且可读写返回TRUE
    5) is_executable() => 判断给定文件名是否可执行
        参数是文件名, 如文件存在且可执行返回TRUE
    6) filectime() => 获取文件的创建时间
        参数是文件名, 返回UNIX时间戳格式
    7) filemtime() => 获取文件的修改时间
        参数是文件名, 返回UNIX时间戳格式
    8) fileatime() => 获取文件的访问时间
        参数是文件名, 返回UNIX时间戳格式
    9) stat() => 获取文件大部分属性值
        参数是文件名, 返回关于给定文件有用信息的数组




## **3. 目录的基本操作** ##
> ### **3.1 解析目录路径** ###
    1) basename
        返回路径中的文件名部分;
        string basename(string path[, string suffix]);
    2) dirname
        返回去掉文件名后的目录名; 给出一个包含有指向一个文件的全路径字符串;
    3) pathinfo
        返回一个关联数组, 其中包含指定路径中的目录名, 基本名或扩展名
> ### **3.2 遍历目录** ###
    1) opendir => 打开指定目录
    2) readdir => 读取指定目录
    3) closedir => 关闭指定目录
    4) rewinddir => 倒回目录句柄
> ### **3.3 统计目录大小** ###
    可以使用disk_free_space和disk_total_space两个函数实现
> ### **3.4 建立和删除目录** ###
    1) mkdir只需要传入一个目录名即可很容易建立新目录
    2) rmdir只能删除一个空目录并且目录必须存在
    3) 非空目录, 需要进入目录, 用unlink删除每个文件, 再删除空目录
> ### **3.5 复制或移动目录** ###
    copy函数可以实现, 如果需要复制目录下有多个子目录, 就需要递归复制




## **4. 文件的基本操作** ##
> ### **4.1 文件的打开和关闭** ###
    1) fopen
        resource fopen(string filename, string mode[, 
                      bool use_include_path[, resource zcontext]]);
        r : 只读方式打开, 从文件头开始读
        r+ : 读写方式打开, 从文件头开始读写
        w : 只写方式打开, 从文件头开始写
        w+ : 读写方式打开, 从文件头开始读写
        x : 创建并以写方式打开, 将文件指针指向文件头
        x+ : 创建并以读写方式打开, 将文件指针指向文件头
        a : 写入方式打开, 将文件指针指向文件尾
        a+ : 写入方式打开文件, 将文件指针指向文件尾
        b : 以二进制模式打开文件, 用于与其他模式进行链接
        t : 以文本模式打开文件
    2) fclose
        fclose会撤销fopen打开的文件资源
> ### **4.2 写入文件** ###
        int fwrite(resource handle, string string[, int length]);
> ### **4.3 读取文件内容** ###
    1) fread
        在打开的文件中读取指定长度的字符串, 也可安全用于二进制文件
        string fread(int handle, int length);
    2) fgets, fgetc
        一次至多从打开的文件资源中读取一行内容:
        string fgets(int handle[, int length]);
        fgetc在打开的文件资源中只读取当前指针位置的一个字符
    3) file 
        不需要使用fopen打开文件, 可以将整个文件读入到一个数组中
    4) readfile
        可以读取指定的整个文件, 立即输出到输出缓冲区, 并返回读取字节数
> ### **4.4 访问远程文件** ###
        如需要访问远程文件, 必须在PHP配置文件中激活"allow_url_fopen"选项, 才
    可以使用fopen系列的函数进行操作
> ### **4.5 移动文件指针** ###
    1) int ftell(resource handle) => 返回文件指针的当前位置
    2) int fseek(resource handle, int offset[, int whence]) => 移动文件指针
       到指定的位置
       whence的选项值与C语言类似
    3) bool rewind(resource handle) => 移动文件指针到文件头
> ### **4.6 文件的锁定机制** ###
        bool flock(int handle, in operation[, int& wouldblodk]);
        operation的选项值:
        LOCK_SH => 取得共享锁定(从文件读取数据时使用)
        LOCK_EX => 取得独占锁定(从文件写入数据时使用)
        LOCK_UN => 释放锁定
        LOCK_NB => 附加锁定(如不希望flock在锁定时阻塞, 则应加上该锁)
> ### **4.7 文件的基本操作函数** ###
    1) copy => 复制文件
        copy(来源文件, 目标文件)
    2) unlink => 删除文件
        unlink(目标文件)
    3) ftruncate => 将文件截断到指定的长度
        ftruncate(目标文件资源, 截取长度)
    4) rename => 重命名文件或目录
        rename(旧文件名, 新文件名)




## **5. 文件的上传和下载** ##
    暂时还没有需求, 当需要的时候再来查阅资料(11章)
