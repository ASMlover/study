# **函数功能模块** #
***

## **1. 日期和时间** ##
> ### **1.1 UNIX时间戳** ###
        UNIX时间戳是保存日期和时间的一种紧凑简洁的方法, 是大多数UNIX系统中保
    存当前日期和时间的一种方法, 也是在大多数计算机语言中表示日期和时间的一种
    标准格式。
    1) 将日期和时间转换为UNIX时间戳
        int mktime([int hour[, int minute[, int second[, 
                   int month[, int day[, int year]]]]]]);
    2) 日期的计算
        计算两个日期之间相隔的长度, 最简单的方法就是通过计算两个UNIX时间戳之
        差来获得。
> ### **1.2 在PHP中获取日期和时间** ###
        time获取当前的UNIX时间戳, getdate获取当前时间, gettimeofday获取某一
    天的具体时间, date_sunrise获取某天日出时间, date_sunset获取某天日落时间
> ### **1.3 时期和时间格式化输出** ###
    1) a : 小写的上午值和下午值
    2) A : 大写的上午值和下午值
    3) d : 月份中的第几天, 有前导零的两位数字
    4) D : 星期中的第几天, 三个字母文本表示
    5) F : 月份完整的文本表示格式
    6) g : 小时, 12小时格式, 没有前导零
    7) G : 小时, 24小时格式, 没有前导零
    8) h : 小时, 12小时格式, 有前导零 
    9) H : 小时, 24小时格式, 有前导零
    10) i : 有前导零的分钟数
    11) I : 是否为夏令时
    12) j : 月份中的第几天, 没有前导零
    13) l : 星期几, 完整的文本格式
    14) L : 是否为闰年
    15) m : 数字表示的月份, 有前导零
    16) M : 三个字母表示的月份
    17) n : 数字表示的月份, 没有前导零
    18) O : 与格林威治时间相差的小时数
    19) r : RFC 822格式的日期
    20) s : 秒数, 有前导零
    21) S : 每月天数后面的英文后缀, 2个字符
    22) t : 给定月份所应有的天数
    23) T : 本机所在的时区
    24) U : UNIX纪元以来的秒数
    25) w : 星期中的第几天, 数字表示 
    26) W : 一年中的第几周(ISO-8601格式年份中)
    27) Y : 四位数字完整表示的年份
    28) z : 年份中的第几天
    19) Z : 时差偏移量的秒数
> ### **1.4 修改PHP默认时区** ###
    1) 在php.ini中修改
        date.timezone=Etc/GMT-8 设置默认时区为东8区
    2) date_default_timezone_set('PRC'); 在输出时间之前设置时区
> ### **1.5 使用微妙计算PHP脚本执行时间** ###
        microtime返回当前UNIX时间戳和微秒数
        mixed microtime([bool get_as_float]);





## **2. 动态图像处理** ##
> ### **2.1 PHP中GD库的使用** ###
    1) 创建一个图像的4个基本步骤
        创建画布 -> 绘制图像 -> 输出图像 -> 释放资源
> ### **2.2 画布管理** ###
    1) 创建画布
        新建基于调色板的图像:
          resource imagecreate(int $x_size, int $y_size); 
        新建一个真彩色的图像:
          resource imagecreatetruecolor(int $x_size, int $y_size); 
        从JPEG文件或URL新建一个图像:
          resource imagecreatefromjpeg(string $filename);
        从PNG文件或URL新建一个图像:
          resource imagecreatefrompng(string $filename);
        从GIF文件或URL新建一个图像:
          resource imagecreatefromgif(string $filename);
        从WBMP文件或URL新建一个图像:
          resource imagecreatefromwbmp(string $filename);
        当画布引用句柄不再使用时一定要释放资源:
          bool imagedestroy(resource $image);
> ### **2.3 设置颜色** ###
        原型:
        int imagecolorallocate(resource $image, 
                                int $red, int $green, int $blue);
        如果需要设置多种颜色, 则调用多次该函数即可
> ### **2.4 生成图像** ###
        将动态绘制完成的画布, 直接生成GIF, JPEG, PNG和WBMP;
        bool imageif(resource $image[, string $filename]);
        bool imagejpeg(resource $image[, string $filename[, int $quality]]);
        bool imagepng(resource $image[, string $filename]);
        bool imagewbmp(resource $image[, string $filename[, int $fg]]);
> ### **2.5 绘制图像** ###
    1) 图形区域填充
        bool imagefill(resource $image, iint $x, int $y, int $color);
        相对图像左上角(0, 0)坐标, 从($x, $y)用$color对区域填充;
    2) 绘制点和线
        在画布中绘制一个单一像素的点:
        bool imagesetpixel(resource $image, int $x, int $y, int $color);
        在画布上绘制一条线段:
        bool imageline(resource $image, int $x1, int $y1, 
                        int $x2, init $y2, int $color);
    3) 绘制矩形
        bool imagerectangle(resource $image, int $x1, int $y1, 
                            int $x2, int $y2, int $color);
        bool imagefilledrectangle(resource $image, int $x1, int $y1, 
                                  int $x2, int $y2, int $color);
    4) 绘制多边形
        bool imagepolygon(resource $image, array $points, 
                          int $num_points, int $color);
        bool imagefilledpolygon(resource $image, array $points, 
                                int $num_points, int $color);
    5) 绘制椭圆
        bool imagellipse(resource $image, int $x, int $y, 
                          int $width, int $height, int $color);
        bool imagefilledellipse(resource $image, int $x, int $y, 
                                int $width, int $height, int $color);
    6) 绘制弧线
        bool imagearc(resource $image, int $x, int $y, int $width, 
                      int $height, int $start, int $end, int $color);
        $start和$end分别指定起始角度和结束角度
> ### **2.6 在图像中绘制文字** ###
    1) 水平画一行字符串
        bool imagestring(resource $image, int $font, int $x, int $y, 
                          string $string, int $color);
    2) 垂直画一行字符串
        bool imagestringup(resource $image, int $font, int $x, int $y, 
                            string $string, int $color);
    3) 水平画一个字符
        bool imagechar(resource $image, int $font, int $x, int $y, 
                        char $c, int $color);
    4) 垂直画一个字符
        bool imagecharup(resource $image, int $font, int $x, int $y, 
                          char $c, int $color);
    5) 输出一种可缩放的与设备阿无关的TrueType字体
        array imagettftext(resource $image, float $size, float $angle, 
                            int $x, int $y, int $color, 
                            string $fontfile, string $text);
