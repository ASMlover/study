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
        在最后添加NDK_ROOT=/cygdrive/d/SDK-Tool/android/android-ndk-r9
        export NDK_ROOT  
 * 对于/home目录下什么都没有的情况, 先将windows环境的HOME删除, 重启cygwin



## **4. 安装android-sdk** ##
 * [android sdk](http://developer.android.com/sdk/index.html)
 * 运行SDK Manager.exe更新sdk 
 * 新建环境变量ANDROID_SDK, 值为: 
 
        D:\SDK-Tool\android\sdk\platforms;D:\SDK-Tool\android\sdk\tools 
 * 在PATH环境变量中加入: %ANDROID_SDK%



## **5. 安装android-ndk** ##
 * [android-ndk](http://developer.android.com/tools/sdk/ndk/index.html)
 * 解压到对应目录即可



## **6. 安装android开发插件** ##
 * 打开eclipse, help -> Install New Software
 * 点击Add
 * Name: Android, Location: https://dl-ssl.google.com/android/eclipse/
 * 全部选中, Next -> Next
 * 选择I accept the terms of the license agreements, Next
 * 安装完成之后重启eclipse
 * 配置ADT, window -> preferences

        SDK Location: D:\SDK-Tool\android\sdk 







# **linux下开发环境配置** #
***


## **1. 下载编译cocos2d-x** ## 
 * 下载cocos2d-2.0-x-2.0.4
 * [下载地址](http://cocos2d-x.googlecode.com/files/cocos2d-2.0-x-2.0.4.zip)
 * 解压 
 * 运行./make-all-linux-project.sh
