# **Ubuntu系统更新(软件更新)** #
***


## **1. apt的使用** ##
    `apt-cache search`              packaage搜索包
    `apt-cache show`                packaage获取包相关信息
    `apt-get install`               packaage安装包
    `apt-get install --reinstall`   重新安装包
    `apt-get -f install`            强制安装
    `apt-get remove`                packaage删除包
    `apt-get remove --purge`        删除包(包括配置文件等)
    `apt-get autoremove --purge`    删除包及其依赖的软件包 + 配置文件
    `apt-get update`                更新源
    `apt-get upgrade`               更新已安装的包
    `apt-get dist-upgrade`          升级系统
    `apt-get dselect-upgrade`       使用dselect升级
    `apt-cache depends`             了解包使用依赖
    `apt-cache rdepends`            察看包被哪些包依赖
    `apt-get build-dep`             安装相关的编译环境
    `apt-get source`                下载该包的源代码
    `apt-get clean && apt-get autoclean`      清理下载文件的存档&&清理过时包
    `apt-get check`                 检查是否有损坏的依赖
