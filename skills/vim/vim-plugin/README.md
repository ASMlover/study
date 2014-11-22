# **README for vim-plugin**
***

## **Summary**
 * plugins for vim


## **Plugins**
> ### **grep.vim**
    1) 将grep.vim拷贝到~/.vim/plugin
    2) 在.vimrc中设置映射快捷键
        nnoremap <silent> <C-F3> :Grep<CR>
    3) 常用命令
        * :Grep 在当前打开的文件中搜寻
        * :Rgrep 在目录中递归搜寻
        * :GrepBuff 在Vim的buff文件中搜寻

> ### **ack.vim**
    1) 安装ack.vim插件
        $ cp plugin/ack.vim ~/.vim/plugin/
        $ cp doc/ack.txt ~/.vim/doc/
    2) Linux安装ack命令
        $ yum install ack
    3) Ubuntu安装ack
        由于debian系列ack是一个其他的命令, 所以需要下载ack-grep
        $ apt-get install ack-grep
        $ ln -s /usr/bin/ack-grep /usr/bin/ack
    4) 在vim中直接使用Ack命令来进行查找
        :Ack [pattern]

> ### **ag.vim**
    1) 安装ag.vim插件
        $ cp autoload/ag.vim ~/.vim/autoload/
        $ cp doc/ag.txt ~/.vim/doc/
        $ cp plugin/ag.vim ~/.vim/plugin/
    2) 源码安装ag命令
        $ git clone https://github.com/ggreer/the_silver_searcher ag
        $ ./build.sh
        $ sudo make install
    3) 直接安装ag命令
        参见the_silver_searcher项目的README
    4) 在vim中直接使用Ag命令来进行查找
        :Ag [pattern]

> ### **vimtweak**
    1) 该插件只针对windows有效果
    2) 安装
        \> cp vimtweak.dll /vim/vim73/
    3) 配置
        nnoremap <silent> <F11> :call libcallnr("vimtweak.dll", "EnableMaximize", 1)<CR>
        nnoremap <silent> <F12> :call libcallnr("vimtweak.dll", "SetAlpha", 240)
