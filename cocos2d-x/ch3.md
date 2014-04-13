# **cocos2d-x中的核心类**
***

    * 导演(Director): 
      是游戏中的组织者和领导者, 是整个游戏的负责人, 总指挥; 可以定制游戏的运
      行规则, 从而让游戏内的场景, 布景类和精灵类有序地进行;
    * 场景(Scene):
      场景就是一个关卡, 或者是一个游戏界面; 这样一个个场景就确定了整个游戏;
    * 布景层(Layer):
      一个场景可以由多个Layer构成, Layer就是关卡里的背景, 关卡不同也就是场景
      需要的Layer不同; 
      有时候, 为了游戏的不同模块的管理更加方便, 会把一个场景分成多个Layer; 
      游戏游戏需要更加细致的细分, 可以分为游戏对象Layer和游戏地图Layer;
    * 人物精灵(Sprite):
      人物精灵可以分为玩家控制的主角类, 敌人类; 
      更复杂的游戏可以分为NPC类, 机关类; 他们是构成游戏的关键



## **1. 节点类**
          Ref
           ^
           |
          Node
    Node不自带贴图, 在屏幕上看不到任何节点类的效果, 一般使用Node有两种情况:
    一是需要一个父节点来管理一批子节点, 这时候可设置一个无形的子节点来管理
    子节点; 二是需要自己定义一个在屏幕上显示的对象, 这时候让新定义的这个类继
    承自Node;
> ### **1.1 坐标系**
    1) OpenGL坐标系
        原点在屏幕左下角, x轴向右, y轴向上;
    2) 世界坐标系
        世界坐标系和OpenGL坐标系方向一致;
    3) 锚点
        指定了贴图上和所在节点原点重合的点的位置;
        锚点的默认值是(0.5, 0.5), 表示的并不是一个像素点, 而是一个乘数因子, 
        (0.5, 0.5)表示锚点位于贴图长度乘0.5和宽乘0.5的地方;
    4) 节点坐标系
        Node类的设置位置使用的就是父节点的节点坐标系, 它和OpenGL坐标系的方向
        也是一致的, x轴向右, y轴向上, 原点在父节点的左下角;
    5) 仿射变化
        是指在线性变化的基础上加上平移;



## **导演类**
       Ref
        ^
        |
      Director
    是cocos2d-x引擎的核心, 用来创建并控制主屏幕的显示, 同时控制场景的显示时
    间和显示方式;
> ### **Director的功能**
    1) 初始化OpenGL会话
    2) 设置OpenGL的一些参数和方式
    3) 访问和改变场景以及访问cocos2d-x的配置细节
    4) 访问视图
    5) 设置投影和朝向



## **场景类**
        Ref
         ^
         |
        Node
         ^
         |
       Scene
    Scene和Node相比只添加了一个特性, 就是拥有自己的锚点, 位置在屏幕正中央;
> ### **切换场景**
    1) 调用Director::getInstance()->purgeCacheData()清空缓存;
    2) 创建新场景;
    3) 调用Director::getInstance()->replaceScene(this)将新创建的场景替换为当
       前场景;
    正常的无过渡场景, 如果是游戏的第一个场景, 使用runWithScene; 如果是替换场
    景使用replaceScene; 如果要使用场景间的切换效果, 则需要使用相应的切换类(
    TransitionScene子类的create函数生成相应的场景); 然后使用replaceScene启动
    场景, 然后再启动;
> ### **切换场景动画类**
        Ref
         ^
         |
        Node
         ^
         |
       Scene
         ^
         |
      TransitionScene
    TransitionCrossFade       淡出淡入交叉, 同时进行
    TransitionFade            淡入淡出, 原场景淡出, 新场景淡入
    TransitionFadeTR          向右上波浪
    TransitionFadeBL          向左下波浪
    TransitionFadeUp          向上百叶窗
    TransitionFadeDown        向下百叶窗
    TransitionFlipX           x轴平移移动
    TransitionFlipY           y轴平移移动
    TransitionFlipAngular     水平角度翻转
    TransitionJumpZoom        跳跃式, 原场景先缩小, 新场景跳跃进来
    TransitionMoveInL         新场景从左移入覆盖
    TransitionMoveInR         新场景从右移入覆盖
    TransitionMoveInT         新场景从上移入覆盖
    TransitionMoveInB         新场景从下移入覆盖
    TransitionPageTurn        翻页
    TransitionRotoZoom        转角切换
    TransitionShrinkGrow      交错切换
    TransitionSlideInL        新场景从左移入推出原场景
    TransitionSlideInR        新场景从右移入推出原场景
    TransitionSlideInT        新场景从上移入推出原场景
    TransitionSlideInB        新场景从下移入推出原场景
    TransitionSplitRows       按行切换
    TransitionSplitCols       按列切换
    TransitionTurnOffTiles    随机小方块切换
    TransitionZoomFlipX       带缩放效果的x轴平移
    TransitionZoomFlipY       带缩放效果的y轴平移
    TransitionZoomFlipAngular 带缩放效果的y轴平移 
> ### **切换场景动画的步骤**
    1) 新建场景
    2) 根据需要的新建场景的切换动画选择TransitionScene子类, 通过create将之前
       建的场景传入其中, 并设置其他参数
    3) 调用Director::getInstance()->replaceScene替换新场景



## **布景层**
        Ref
         ^
         |
        Node
         ^
         |
        Layer
    Layer可以处理输入, 包括触屏和加速度传感器;
