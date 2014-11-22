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
