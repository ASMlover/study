# **windows下开发环境配置** #
***


## **1. 安装JDK** ##
 * [JDK](http://www.oracle.com/technetwork/java/javase/downloads/index.html)
 * 安装完成之后需要配置环境变量

        1) 新建环境变量: JAVA_HOME, 值为D:\SDK-Tool\Java\jdk1.7.0_25
        2) 新建环境变量: CLASSPATH, 值为;%JAVA_HOME%\lib;
        3) 在path变量中添加: %JAVA_HOME%\bin;
        4) 验证, 在cmd窗口中输入java -version


## **2. 安装eclipse** ##
 * [eclipse](http://www.eclipse.org/downloads/)
 * 将下载好的eclipse解压到电脑指定目录中即可 



## **3. 安装cygwin** ##
 * [cygwin](http://www.cygwin.com/)
 * 默认选择安装上Devel即可
 * 执行cygwin.bat, 并修改目录下新生成的.bash_profile文件

        android-ndk-r8是ndk的路径
        在最后添加NDK_ROOT=/cygdrive/d/android-ndk-r8
        export NDK_ROOT
