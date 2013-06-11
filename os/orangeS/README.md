# **README for orangeS** #
***


## **说明** ##
        这里主要记录了从头开始写一个五脏俱全的微型操作系统的相关知识点, 并且
    有一些相关的的demo



## **工具** ##
    1) 这里的操作系统主要使用到了bochs虚拟机, nasm汇编编译器以及gcc编译器等
    2) nasm的安装
        * sudo apt-get install nasm
    3) bochs的安装, 我们可以直接安装二进制包, 但是为了可以调试, 我们选择从源
       代码开始安装(这里安装的是bochs-2.4.5)
        * sudo apt-get install build-essential
        * sudo apt-get install xorg-dev
        * sudo apt-get install libgtk2.0-dev
        * tar vxzf bochs-2.4.5.tar.gz 
        * cd bochs-2.4.5/
        * ./configure --enable-debugger --enable-disasm
        * make
        * sudo make install
