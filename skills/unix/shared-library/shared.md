# **Unix下的共享库** #
***

## **1. Linux下创建共享库** ##
    1) 使用gcc命令如下:
        $ gcc -o $*.o -c -fPIC $^
        $ gcc -o $@ $^ -shared
    2) 具体例子请参见./shared-demo/

## **2. 共享库的使用** ##
    1) gcc -o $@ $^ -lxxx
    2) 在具体运行的时候还需要设置环境变量export LD_LIBRARY_PATH=/xxx/xx/xx
    3) 具体例子请参见./shared-demo/

## **`error while loading shared libraries: xxx.so.x`解决方法**
    1) 原因：
        * OS中没有包含该共享库(lib*.so.*)或版本不对，只能到网上下载
        * 已安装，但执行需要调用该共享库的程序时，程序按默认共享库路径找不到
    2) 解决方法：
        * 如果安装到了/lib或/usr/lib，需要执行ldconfig
          在(/lib和/usr/lib)以及so配置文件/etc/ld.so.conf内所列出的目录下，
          搜寻共享库，创建出动态库所需的链接和缓存文件。默认缓存文件为
          /etc/ld.so.cache，此文件保存已排好序的动态库名字列表
        * 如果共享库安装到了非(/lib和/usr/lib)目录，在执行ldconfig之前还需要
          将新共享库目录加入到共享库配置文件/etc/ld.so.conf中
            # echo "/usr/local/lib" >> /etc/ld.so.conf
            # ldconfig
        * 如果在其他目录，又不想在/etc/ld.so.conf添加路径，那需要export一个
          全局变量LD_LIBRARY_PATH
            $ export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
