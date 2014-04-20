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
