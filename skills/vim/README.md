# **README for vim** #
***


## **说明** ##
    主要记录vim的相关操作技巧以及环境等...
    vim-demo是一个终端下的vim配置例子

    win-vim主要记录了windows下面gvim的配置信息



## **将vim中关键字设置成非粗体模式**
    需要修改colorscheme对应的主题模式中的代码;
    比如修改desert.vim 
    在文件的最后添加上:
    hi Type gui=none
    hi Statement gui=none


## **Vim退格建失效的解决方案**
    具体请参见VIM帮助 :h bs

    backspace的取值如下:
    indent  允许在自动缩进上退格
    eol     允许在换行符上退格(连接行)
    start   允许在插入开始的位置上退格;C-W/C-U到达插入开始的位置时停留一次
    如果backspace的取值为空, 表示和vi兼容

    为了和5.4以及更早的版本兼容
    0 => :set backspace=
    1 => :set backspace=indent,eol 
    2 => :set backspace=indent,eol,start 

    所以解决方法就是在.vimrc文件中加入如下代码:
    set backspace=indent,eol,start
