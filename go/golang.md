# **Knowledge for Golang** #
***


## **1. 安装** ##
> ### **1.1 Linux** ###
    $ sudo apt-get install go 
> ### **1.2 Windows** ###
    1) 下载地址https://code.google.com/p/go/downloads/list
    2) 选择对应的系统架构对应的安装包安装即可 
> ### **1.3 判断安装成功没有** ###
    // hello.go 
    package main 

    func main() ｛
      println("hello, world!")
    }
    1) Linux下直接在命令行下运行:
        $ go run hello.go 
    2) Windows下直接在命令行下运行:
        \> go run hello.go 


## **2. VIM配置** ##
> ### **2.1 Linux** ###
    将$GOROOT/misc/vim/syntax/go.vim拷贝到~/.vim/syntax/和~/.vim/ftdetect/
> ### **2.2 Windows** ###
    将$GOROOT/misc/vim/syntax/go.vim拷贝到gvim安装目录下的vimfiles/syntax/以
    及gvim安装目录下的vimfiles/ftdetect/
