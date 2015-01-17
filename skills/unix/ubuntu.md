# **Ubuntu Skills**
***


## **Summary**
 * 记录一些使用Ubuntu的经验和技巧



## **修改Grub2启动选项**
    $ vim /etc/default/grub 
    $ update-grub 
    其中涉及很多启动的选项, 根据各个选项配置来修改grub, 从而达到我们的目的;
    修改配置后一定要运行update-grub, 否则不会其作用



## **update需要公钥**
    在进行sudo apt-get update的时候出现:
      由于没有公钥, 无法验证下列签名: NO_PUBKEY D45DF2E8FC91AE7E
    
    解决方法:
      $ gpg --keyserver subkeys.pgp.net --recv D45DF2E8FC91AE7E
      $ gpg --export --armor D45DF2E8FC91AE7E | sudo apt-key add -



## **升级gcc**
    1. 更新ubuntu源
        $ sudo add-apt-repository ppa:ubuntu-toolchain-r/test
        $ sudo apt-get update
        $ sudo apt-get install gcc-4.8
        $ sudo apt-get install g++-4.8
    2. 建立链接到gcc-4.8
        $ cd /usr/bin/ 
        $ sudo rm gcc 
        $ sudo rm g++
        $ sudo ln -s gcc-4.8 gcc 
        $ sudo ln -s g++-4.8 g++


## **Linux映射CapsLock为Control**
    1. 请使用xmodmap查看系统的键盘映射情况:
        $ xmodmap -pm
    2. 在当前用户目录下~/添加.Xmodmap文件, 内容如下:
        remove Lock = Caps_Lock
        keysym Caps_Lock = Control_L
        add lock = Caps_Lock
    3. 在.bashrc文件的尾部添加:
        xmodmap ~/.Xmodmap
    4. 重新启动系统或执行如下命令:
        $ source ~/.bashrc
