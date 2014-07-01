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



## **7. cygwin下编译错误处理** ##
 * cc1plus.exe: fatal error: jni/hellocpp/main.cpp: Permission denied
 * 这个是win7下权限问题造成的
 * 以管理员身份运行cmd, 并指定到工程目录
 * 修改文件和目录的权限: takeown /f * /a /r 
 * 授权到everyone组: icacls * /t /grant:r everyone:f 
 * some warnings being treated as errors

        * 在proj.android/jni目录的Application.mk文件中添加一行
        * APP_CFLAGS += -Wno-error=format-security






# **linux下开发环境配置** #
***


## **1. 下载编译cocos2d-x** ##
 * 下载cocos2d-2.0-x-2.0.4
 * [下载地址](http://cocos2d-x.googlecode.com/files/cocos2d-2.0-x-2.0.4.zip)
 * 解压 
 * 运行./make-all-linux-project.sh







# **WIN7下cocos2d-x3.0环境配置**
***

## **1. 工具准备**
    * Visual Studio(2012+)
    * Python(2.7.x)
    * cocos2d-x[下载](http://cocostudio.download.appget.cn/Cocos2D-X/3.0/cocos2d-x-3.0.zip)


## **2. 安装配置**
  * `安装Python, 并加入环境变量`
  * `安装cocos2d-x3.0, 运行setup.py, 将COCOS_CONSOLE_ROOT加入环境变量`
  * `cocos命令在cocos2d-x-3.0/tools/cocos2d-console/bin下`
  * `命令行运行:`
    `cocos new [PROJ_NAME] -p com.Company.[PROJ_NAME] -l cpp -d [PROJ_DIR]`
  * 安装配置[JAVA JDK](http://www.oracle.com/technetwork/java/javase/downloads/index.html)
    - `JAVA_HOME=D:\SDK-Tool\Java\jdk1.8.0_05`
    - `CLASSPATH=.;%JAVA_HOME%\lib;`
    - `Path新增%JAVA_HOME%\bin;`
  * 下载[Android SDK](http://developer.android.com/sdk/index.html)解压
    - `--<adt-bundle-windows-x86>`
    - `--<eclipse>`
    - `--<sdk>`
    - `--SDK Manager.exe`
  * 下载[Android NDK](http://developer.android.com/tools/sdk/ndk/index.html)解压
    - `--<adt-bundle-windows-x86>`
    - `--<eclipse>`
    - `--<sdk>`
    - `--<android-ndk-r9e>`
    - `--SDK Manager.exe`
  * 下载[ANT](http://ant.apache.org/bindownload.cgi)解压
    - `--<adt-bundle-windows-x86>`
    - `--<eclipse>`
    - `--<sdk>`
    - `--<android-ndk-r9e>`
    - `--<apache-ant-1.9.4>`
    - `--SDK Manager.exe`
  * 环境变量
    - `NDK_ROOT=D:\adt-bundle-windows-x86\android-ndk-r9e`
    - `ANDROID_SDK_ROOT=D:\adt-bundle-windows-x86\sdk`
    - `ANT_ROOT=D:\adt-bundle-windows-x86\apache-ant-1.9.4\bin`
    - `ANDROID_SDK=.;%ANDROID_SDK_ROOT%\platforms;%ANDROID_SDK_ROOT%\tools;%ANDROID_SDK_ROOT%\platform-tools`
    - `在Path前新增ANDROID_SDK`



## **3. 多个层之间切换**
    可以使用LayerMultiplex来管理各个Layer;
        auto layer1 = Layer1::create();
        auto layer2 = Layer2::create();
        layer_multi = LayerMultiplex::create(layer1, layer2, nullptr);
        ...
        layer_multi->switchTo(1);



## **4. cocos2dx创建滚动层**
    使用cocostudio创建一个ScrollView层, 然后将层的大小设置为x * x, 然后将 
    ScrollView的滚动高设置为大于x的数字, 再选中裁剪;
    这样在显示的时候就只会显示x区域的大小, 而上下滑动就可以实现层上的东西上
    下滚动了;



## **5. 屏幕适配**
  * 先设置当前窗口的大小glview->setFrameSize(442, 640);
  * 在设置设计上的分辨率setDesignResolutionSize(640, 960, ResolutionPolicy::SHOW_ALL);

    setDesignResolutionSize的第三个参数如下:
      * ResolutionPolicy::EXACT_FIT     缩放拉伸全屏
      * ResolutionPolicy::NO_BORDER     不显示操作面板
      * ResolutionPolicy::SHOW_ALL      显示全部, 但保持高宽比
      * ResolutionPolicy::FIXED_HEIGHT  保持高度不变
      * ResolutionPolicy::FIXED_WIDTH   保持宽度不变
      * ResolutionPolicy::UNKNOWN



## **6. 坐标系**
    1) 屏幕坐标系(UI坐标系)
        --------------> x
        |
        |
        |
        |
        V y
    2) OpenGL坐标系
        ^ y
        |
        |
        |
        |
        --------------> x
    3) UI坐标系和OpenGL坐标系相互转换
        Point Director::getInstance()->convertToGL(Point& point);
        Point Director::getInstance()->convertToUI(Point& point);
    4) 世界坐标系
        绝对坐标系, 原点与OpenGL坐标系一致; 世界就是游戏世界, 建立了描述其他
        坐标系所需要的参考标准;
    5) 节点坐标系(相对坐标系)
        和特定的节点相关联的坐标系; 每个节点都有独立的坐标系;
        节点移动或改变方向时, 和该节点关联的坐标系也随着移动或改变方向;
        Node设置位置就是父节点的节点坐标系, 方向与OpenGL坐标系方向相同;
    6) 锚点
        指定贴图上和所在节点原点(设置位置的点)重合的点的位置, 只有Node设置了
        贴图锚点才有意义, 默认为(0.5, 0.5);
        相对节点而言的, 是节点的一个属性, 是节点位置的一个参基准点, 只影响节
        点在屏幕上的渲染位置;
