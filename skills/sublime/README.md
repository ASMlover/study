# **README for SublimeText**
***


## **1. Windows安装**
    解压安装包并安装, 破解即可...


## **2. 安装格式化插件**
    1) Shift+Ctrl+p -> Install Package -> Aligment
    2) 选中 -> Ctrl+Alt+A


## **3. 常用快捷键**
    1) 选择类
        * Ctrl+D 选择光标所占文本, 继续则选中下一个相同文本
        * Alt+F3 选择文本按下快捷键, 可一次性对全部相同文本同时编辑
        * Ctrl+L 选中整行, 继续则选择下一行
        * Ctrl+Shift+L 选中多行按下快捷键, 会在每行尾插入光标, 可同时编辑
        * Ctrl+Shift+M 选择括号内的内容
        * Ctrl+M 光标移动到括号内结束或开始的位置
        * Ctrl+Enter 在下一行插入新行
        * Ctrl+Shift+Enter 在上一行插入新行
        * Ctrl+Shift+[ 选择代码, 按下快捷键, 折叠代码
        * Ctrl+Shift+] 选择代码, 按下快捷键, 展开代码
        * Ctrl+K+0 展开所有折叠代码
        * Ctrl+<- 向左单位性移动光标, 快速移动光标
        * Ctrl+-> 向右单位性移动光标, 快速移动光标
        * Shift+向上键 向上选择多行
        * Shift+向下键 向下选择多行
        * Shift+<- 向左选择文本
        * Shift+-> 向右选择文本
        * Ctrl+Shift+<- 向左单位性选择文本
        * Ctrl+Shift+-> 向右单位性选择文本
        * Ctrl+Shift+向上键 将光标所在行和上一行代码互换
        * Ctrl+Shift+向下键 将光标所在行和下一行代码互换
        * Ctrl+Alt+向上键 向上添加多行光标, 可同时编辑多行
        * Ctrl+Alt+向下键 向下添加多行光标, 可同时编辑多行
    2) 编辑类
        * Ctrl+J 合并选择的多行为一行
        * Ctrl+Shift+D 复制光标所在行, 插入到下一行
        * Tab 向右缩进
        * Shift+Tab 向左缩进
        * Ctrl+K+K 从光标处开始删除代码至行尾
        * Ctrl+Shift+K 删除整行
        * Ctrl+/ 注释单行
        * Ctrl+Shift+/ 注释多行
        * Ctrl+K+U 转换大写
        * Ctrl+K+L 转换小写
        * Ctrl+Z 撤销
        * Ctrl+Y 恢复撤销
        * Ctrl+U 软撤销, 感觉和Ctrl+Z一样
        * Ctrl+F2 设置书签
        * Ctrl+T 左右字母互换
        * F6 单词检测拼写
    3) 搜索类
        * Ctrl+F 打开底部搜索框, 查找关键字
        * Ctrl+Shift+F 在文件夹内查找
        * Ctrl+P 打开搜索框,
          1. 输入文件名, 快速搜索文件
          2. 输入@和关键字, 快速定位到文件中函数
          3. 输入:和数字, 跳转到文件中该行
          4. 输入#和关键字, 快速定位到变量
        * Ctrl+G 打开搜索框, 自带:, 输入数字, 跳转到文件中该行
        * Ctrl+R 打开搜索框, 自带@, 输入关键字, 快速定位到文件中函数
        * Ctrl+: 打开搜索框, 自带#, 输入关键字, 快速定位到变量
        * Ctrl+Shift+P 打开命令框
        * Esc 退出光标多行选择, 退出搜索框, 命令框...
    4) 显示类
        * Ctrl+Tab 按文件浏览过的顺序, 切换当前窗口的标签页
        * Ctrl+PageDown 向左切换当前窗口的标签页
        * Ctrl+PageUp 向右切换当前窗口的标签页
        * Alt+Shift+1 窗口分屏, 恢复默认1屏(非小键盘数字)
        * Alt+Shift+2 左右分屏 -> 2列
        * Alt+Shift+3 左右分屏 -> 3列
        * Alt+Shift+4 左右分屏 -> 4列
        * Alt+Shift+5 等分4屏
        * Alt+Shift+8 垂直分屏 -> 2屏
        * Alt+Shift+9 垂直分屏 -> 3屏
        * Ctrl+K+B 开启/关闭侧边栏
        * F11 全屏模式
        * Shift+F11 免打扰模式


## **4. 常用插件**
    1) All Autocomplete 自动补全, 自动匹配所有打开的文件的词条;
    2) Markdown Preview
    3) IMESupport 修复中文输入法输入框不跟随的问题
    4) LuaAutocompelete lua自动补全

## **5. 常用配置**
    [Preferences] -> [Setting-User]
```json
{
    "bold_folder_labels": true,                 // 文件目录设置为粗体
    "default_line_ending": "unix",              // Unix风格换行符
    "draw_white_space": "all",                  // 显示空白符
    "highlight_line": true,                     // 光标所在行高亮
    "show_encoding": true,                      // 显示文件编码
    "trim_trailing_white_space_on_save": true,  // 保存时删除行尾空白符
}
```
