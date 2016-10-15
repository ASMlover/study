# **Docker**

## **1. docker中启动ubuntu**
    $ docker run -i -t ubuntu /bin/bash

## **2. docker常量命令**
    1、查看docker信息(version, info)
``` bash
    $ docker version
    $ docker info
```
    2、对image操作(search, pull, images, rmi, history)
``` bash
    # 查找image
    $ docker search <image-name>

    # 下载image
    $ docker pull <image-name>

    # 列出镜像列表
    $ docker images

    # 删除一个镜像
    $ docker rmi <image-name>

    # 显示一个镜像的历史
    $ docker history <image-name>
```
    3、启动容器(run)
``` bash
    # 在容器中运行echo命令
    $ docker run <image-name> echo "hello, world"

    # 进入交互容器
    $ docker run -it <image-name> /bin/bash
    # like:
    $ docker run -it ubuntu /bin/bash

    # 在容器中安装新程序
    $ docker run <image-name> apt-get install -y <app-name>
```
    4、查看容器(ps)
``` bash
    # 列出当前正在运行的container
    $ docker ps

    # 显示所有的container
    $ docker ps -a

    # 列出最近一次启动的container
    $ docker ps -l
```
    5、对container的操作(rm, stop, start, kill, logs, diff, top, cp, restart, attach)
``` bash
    # 删除所有的container
    $ docker rm `docker ps -aq`

    # 删除单个container
    $ docker rm <container-id>

    # 启动、停止、杀死一个container
    $ docker start <container-id>
    $ docker stop <container-id>
    $ docker kill <container-id>

    # 从一个container中获取日志
    $ docker logs <container-id>

    # 列出一个container里面被改变的文件或目录
    $ docker diff <container-id>

    # 显示一个运行的container里面的进程信息
    $ docker top <container-id>

    # 从容器中拷贝文件/目录到本地路径
    $ docker cp <container-name>:/<source-path> <target-path>
    $ docker cp <container-id>:/<source-path> <target-path>

    # 从本地路径拷贝文件/目录到container
    $ docker cp <source-path> <container-name>:/<target-path>
    $ docker cp <source-path> <container-id>:/<target-path>

    # 重启一个正在运行的container
    $ docker restart <container-id>

    # 附加到一个运行的容器上
    $ docker attach <container-id>

    # 进入正在运行的container内部，同时运行bash(比attach更好，建议使用它)
    $ docker exec -it <container-id> /bin/bash
```
