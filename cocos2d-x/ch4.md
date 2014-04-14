# **cocos2d-x中的动作, 特效与动画**
***



## **1. 动作类**
        Ref         Clonable
         ^             ^
         |_____________|
                ^
         _______|________________________
         |                |             |
      FiniteTimeAction  Follow        Speed
    Action有3个子类: 有限时间动作, 跟随, 移动速度; 有限时间动作分为随时动作
    (ActionInstant)和延时动作(ActionInterval);
> ### **基本动作**
    FiniteTimeAction分为瞬时动作(ActionInstant)和延时动作(ActionInterval), 
    瞬时动作的子类如下:
      CallFunc            执行回调函数
      FlipX               x轴镜像翻转
      FlipY               y轴镜像翻转
      Hide                隐藏
      Place               设置位置
      ReuseGrid           重用网格
      Show                显示
      StopGrid            停止网格
      ToggleVisibility    可见切换
    延时动作会形成动画过程, 子类如下:
      BezierBy            贝塞尔曲线, 移动固定的距离
      Blink               闪烁
      DelayTime           延时
      FadeTo              变暗
      MoveTo              移动到目的点
      RotateTo            移动到相应的角度
      FadeIn              由无变亮
      FadeOut             由亮变无
      JumpBy              按抛物线轨迹移动相应距离, 参数是相对的距离
      Sequence            动作序列
      Spawn               合并多个动作, 使多个动作同时进行
      ScaleTo             缩放到原来的固定倍数
      TintTo              色调变化到
      TintBy              色调变化相对数值
      Animate             动画
      ReverseTime         时间逆向
      Repeat              有限次重复
      RepeatForever       无限次重复
      ActionEase          变速移动
      ActionCamera        摄像机动作
      SkewTo              扭曲效果到设定的参数
      TargetedAction      动作的目标并不一定是动作的执行者动作
      CardinalSplineTo    基样曲线移动到相应的位置
      DeccelAmplitude     有相应幅度参数的动作, 附带动作时间, 有减速效果
      AccelAmplitude      有相应幅度参数的动作, 附带动作时间, 有加速效果
      AccelDeccelAmplitude有相应幅度参数的动作, 附带动作时间, 有变速效果
      * 移动动作: MoveTo, MoveBy
      * 缩放动作: ScaleTo, ScaleBy
      * 扭曲动作: SkewTo, SkewBy
      * 选择动作: RotateTo, RotateBy
      * 跳跃动作: JumpTo, JumpBy
      * 贝塞尔曲线动作: BezierTo, BezierBy
      * 淡入淡出动作: FadeIn, FadeOut
      * 闪烁动作: Blink(create第一个参数是动作时间, 第二个参数是闪烁次数)
      * 色值渐变动作: TintTo, TintBy
> ### **基本样条动作**
    有时希望使用一些非常规轨迹能描述的运动轨迹, 只是告诉游戏对几个离散的点, 
    游戏对象就可以根据这些离散的点模拟出相应的路径;
    * CardinalSplineTo, CardinalSplineBy拉力系数是0.5
    * CatmullRomTo, CatmullRomBy拉力系数可以自定义
> ### **缓冲动作**
    ActionEase有很多子类, 根据不同的缓冲公式来模拟加减速过程;
    * 指数缓冲: EaseExponentialIn, EaseExponentialOut, EaseExponentialInOut
    * 赛因缓冲: EaseSineIn, EaseSineOut, EaseSineInOut
    * 跳跃缓冲: EaseBouneceIn, EaseBouneceOut, EaseBouneceInOut
    * 弹性缓冲: EaseElasticIn, EaseElasticOut, EaseElasticInOut
    * 回震缓冲: EaseBackIn, EaseBackOut, EaseBackInOut
> ### **组合动作**
    游戏对象有时是多个动作的动作序列, 有时是同时执行几个动作序列;
    * Sequence: 
      定义一个动作序列, 可使用动作的Array数组, 也可以把所有的动作作为参数传
      入create函数, 最后结尾参数使用nullptr即可;
    * Spawn: 使被合成的动作同时进行;
    * Repeat, RepeatForever: 使动作重复播放;
> ### **跟随动作**
    Follow是一个节点跟随另外一个节点的动作;
> ### **可调整速度动作**
    Speed不是一个独立的动作, 是对目前动作的包装, 可以实现慢动作和快动作的效
    果;
> ### **延时动作**
    DelayTime就是动作延后一段固定时间, 可以理解成"空动作", 是有时间, 没有任
    何动作;
> ### **改变动作执行对象**
    TargetedAction可以改变动作的执行对象, 一般默认的动作执行对象是调用
    runAction的对象, 有时候需要子定义动作执行对象, 这时就用TargetedAction;
> ### **函数回调动作**
    需要执行一些数据上的处理(攻击一个敌人, 需要处理加减血); 这时就需要使用函
    数回调动作CallFunc;
    CallFunc就是回调函数, 该回调的函数不含参数, CallFunc的回调函数以Node对象
    和数据作为参数;
> ### **过程动作**
    进入游戏之间需要载入动作, 需要一些动作来实现载入时的动画; ProgressTo和
    ProgressFromTo可以实现这个动画, 但执行这种动作的节点是ProgressTimer;
