# **cocos2d-x中的菜单项和文本渲染系统**
***



## **1. 菜单项**
          Ref
           ^
           |
          Node
           ^
           |
        MenuItem
    MenuItem是一个基类, 它的子类可以加入Menu中形成菜单;
> ### **标签菜单项**
    使用字体定义的菜单项(MenuItemAtlsFont, MenuItemFont); 
    MenuItemFont是通过设定字体名称来设置字体的, 这个字体是系统自带的, 支持的
    字体(Courier New, Marker Felt, American Typewriter, Arial);
    MenuItemAtlsFont通过字体配置PNG文件的LabelAtlas或FNT类型文件的
    LabelBMFont, 即配置文件所对应的图片;
> ### **精灵菜单项**
    MenuItemSprite的特点是可以封装图片进入菜单项;
> ### **触发器菜单项**
    MenuItemToggle可以将任意的菜单项穿进去, 作为一个触发器按钮式的开关;



## **2. 文本渲染类**
    他们都继承自LabelProtocol协议;
                            LabelProtocol
                                 ^
           ______________________|__________________________
           |                |               |              |
        Label           LabelAtlas      LabelBMFont     LabelTTF
> ### **LabelAtlas**
    是使用图片作为文字的一种方式, 可以通过图片直接定义, 还可以使用plist描述;
> ### **LabelTTF**
    是通过系统字体来实现字体标签的;
    TextFieldTTF是LabelTTF类的子类, 是输入框使用的文字标签;
> ### **LabelBMFont**
    也是文字渲染标签类, LabelBMFont类中的每个字都是一个精灵类, 每个字都可以
    有自己的旋转动作, 并支持FNT类型的文件;
