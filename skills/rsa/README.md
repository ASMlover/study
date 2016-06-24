# **rsa**
***

## **1. 利用openssl生成**
    $ openssl
    OpenSSL> genrsa -out private.pem 2048
    OpenSSL> rsa -in private.pem -pubout -out public.pem
    OpenSSL> exit

## **2. 利用ssh-keygen生成**
    $ ssh-keygen
    根据输入的文件名(加入为rsa)会输出rsa以及rsa.pub两个文件
