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







# **开发技巧**
***

## **1. 多个层之间切换**
    可以使用LayerMultiplex来管理各个Layer;
        auto layer1 = Layer1::create();
        auto layer2 = Layer2::create();
        layer_multi = LayerMultiplex::create(layer1, layer2, nullptr);
        ...
        layer_multi->switchTo(1);



## **2. cocos2dx创建滚动层**
    使用cocostudio创建一个ScrollView层, 然后将层的大小设置为x * x, 然后将 
    ScrollView的滚动高设置为大于x的数字, 再选中裁剪;
    这样在显示的时候就只会显示x区域的大小, 而上下滑动就可以实现层上的东西上
    下滚动了;



## **3. 屏幕适配**
  * 先设置当前窗口的大小glview->setFrameSize(442, 640);
  * 在设置设计上的分辨率setDesignResolutionSize(640, 960, ResolutionPolicy::SHOW_ALL);

    setDesignResolutionSize的第三个参数如下:
      * ResolutionPolicy::EXACT_FIT     缩放拉伸全屏
      * ResolutionPolicy::NO_BORDER     不显示操作面板
      * ResolutionPolicy::SHOW_ALL      显示全部, 但保持高宽比
      * ResolutionPolicy::FIXED_HEIGHT  保持高度不变
      * ResolutionPolicy::FIXED_WIDTH   保持宽度不变
      * ResolutionPolicy::UNKNOWN



## **4. 坐标系**
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



## **5. 开启和关闭多点触控**
    1) 在层一创建的时候就添加触摸事件
    2) 添加一个全局的多点触控是否开启的开关
    3) 在多点触控响应函数中判断开关, 做不同的处理
        this->setTouchEnabled(true);
        auto eventDispatcher = 
          Director::getInstance()->getEventDispatcher();
        auto touchListener = EventListenerTouchOneByOne::create();
        if (nullptr != touchListener) {
          touchListener->onTouchBegan = [&](Touch* touch, Event* event) {
            showTouchInfo(_touchLabel, touch, _canTouches);
            return true;
          };
          touchListener->onTouchEnded = [&](Touch* touch, Event* event) {
            showTouchInfo(_touchLabel, touch, true);
          };
          eventDispatcher->addEventListenerWithSceneGraphPriority(
            touchListener, this);
        }
        auto touchesListener = EventListenerTouchAllAtOnce::create();
        if (nullptr != touchesListener) {
          touchesListener->onTouchesBegan 
            = [&](const std::vector<Touch*>& touches, Event* event) {
            showTouchesInfo(_touchesLabel, touches, !_canTouches);
          };
          touchesListener->onTouchesEnded 
            = [&](const std::vector<Touch*>& touches, Event* event) {
            showTouchesInfo(_touchesLabel, touches, true);
          };
          eventDispatcher->addEventListenerWithSceneGraphPriority(
            touchesListener, this);
        }
        


## **6. cocos2d-x处理ListView**
    1) Button上有一些Label等其他UI对象, 将Button加入ListView
    2) 在添加的时候, button->clone()然后再在button基础上载入其他UI对象, 然后
       再添加到ListView中;
    3) ListView的创建后需要setTouchEnabled, setBounceEnabled;
		4) ListView中的Button等Item需要使用insertCustomItem以后点击的时候才能进
			 行相应的背景修改等。。。


## **7. TextField在Google输入法无法及时显示**
    1) 需要TextField在软键盘输入时不弹出全部的输入框
    2) cocos自身处理字符的输入显示, 光标显示和删除的更新
    3) 需要在cocos的platform/android下的Cocos2dxEditText.java中对于IME的模式
       采用EditorInfo.IME_FLAG_NO_FULLSCREEN
        public void setCocos2dxGLSurfaceView(
            final pCocos2dxGLSurfaceView) {
          this.mCocos2dxGLSurfaceView = pCocos2dxGLSurfaceView;
          this.setImeOptions(EditorInfo.IME_FLAG_NO_FULLSCREEN);
        }
