# **README for package**
***


## **说明**
  * Python一些扩展库的安装以及说明;


## **1. lupa**
  * [lupa](https://github.com/scoder/lupa)
  * **Linux**

        $ sudo apt-get install liblua5.2-dev
        $ sudo apt-get install python-pip
        $ sudo pip install lupa

  * **Windows**

        1. 下载LuaJIT
        2. 解压到所下载的lupa目录(lupa-1.1)
        3. 假设本机安装的是VS2012
        4. 编译LuaJIT之后将编译的luajit.exe拷贝到luajit目录, 结构如下:
            + LuaJIT/
              - luajit
              + lua/
                - jit/
            将LuaJIT设置到环境变量
        5. 如果是Python27(设置VC命令行编译环境)
            \> cd LuaJIT-2.0.2\src
            \> msvcbuild.bat static
            \> cd ..\..
            \> set VS90COMNTOOLS=%VS110COMNTOOLS%
            \> python setup.py build
            \> python setup.py install
        6. 如果是Python34(设置VC命令行编译环境)
            \> cd LuaJIT-2.0.2\src
            \> msvcbuild.bat static
            \> cd ..\..
            \> set VS100COMNTOOLS=%VS110COMNTOOLS%
            \> python setup.py build
            \> python setup.py install
