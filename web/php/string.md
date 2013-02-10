# **字符串和正则表达式** #
***

## **1. 常用的字符串输出函数** ##
> ### **1.1 函数echo** ###
    1) 用于输出一个或多个字符串
    2) 效率比其他字符串输出函数高
    3) 该函数的语法格式:
        void echo(string arg1[, string ...]);
> ### **1.2 函数print** ###
    功能和echo一样, 成功返回1, 失败返回0
> ### **1.3 函数die** ###
    1) 该函数是exit函数的别名
    2) 参数是一个字符串, 该函数会在退出前输出它
    3) 参数是一个整数, 会被用作退出状态, 退出状态值[0 ~ 254]
> ### **1.4 函数printf** ###
    1) 输出格式化的字符串, 与C语言一样
    2) 语法格式:
       printf(format, arg1, arg2, ..., argn);
> ### **1.5 函数sprintf** ###
    1) 与printf的格式相似
    2) 将格式化的字符串以返回值的形式写入到一个变量中



## **2. 常用字符串格式化函数** ##
> ### **2.1 去除空格和字符串填补函数** ###
    1) string ltrim(string str[, string charlist]); 
        从左侧删除空格或其他预定义字符
    2) string rtrim(string str[, string charlist]);
        从右侧删除空格或其他预定义字符
    3) string trim(string str[. string charlist]);
        从字符串两端删除空格和其他预定义字符
    4) 这3个函数会将处理后的结果以新字符串的形式返回, 不修改原串
    5) 默认情况下去掉的字符:
        " ": ASCII为32的字符(0x20), 即空格
        "\0": ASCII为0的字符(0x00), 即NULL
        "\t": ASCII为9的字符(0x09), 即制表符
        "\n": ASCII为10的字符(0x0A), 即新行
        "\r": ASCII为13的字符(0x0D), 即回车
    6) 可以使用".."指定需要去除的一个范围, 如"0..9"
    7) 可以使用str_pad按需求对字符串进行填补
        原型:
        string str_pad(string input, 
                       int pad_len[, string pad_str[, int pad_type]]);
        第一个参数指明要处理的字符串; 
        第二个参数是处理后字符串的长度, 如小于原始字符串长度, 则不处理;
        第三个参数指定填补时所用的字符串, 默认使用空格;
        第四个参数指定填补方向=> STR_PAD_BOTH   字符串两端
                                 STR_PAD_LEFT   字符串左端
                                 STR_PAD_RIGHT  字符串右端
> ### **2.2 字符串大小写转换** ###
    1) strtoupper将给定字符串转换为大写
    2) strtolower将给定字符串转换为小写
    3) ucfirst将给定字符串首字母转换为大写
    4) ucwords将给定字符串中全部以空格分隔的单词首字母转换为大写
> ### **2.3 和HTML标签相关的字符串格式化** ###
    1) 函数nl2br()
        在浏览器中输出字符串只能通过HTML的"<br>"标记换行, 而很多人习惯使用
        "\n"作为换行符, 但浏览器不识别, nl2br可以达到该功能。
    2) 函数htmlspecialchars()
        "&"转换为"&ampi;"
        """转换为"&quot;"
        "'"转换为"&#039;"
        "<"转换为"&lt;"
        ">"转换为"&gt;"
        string htmlspecialchars(string string 
                                [, int quote_stype[, string charset]]);
> ### **其他字符串格式化函数** ###
    1) strrev
        将输入的字符反转, 返回反转后的字符串
    2) number_format
        string number_format(float number, [, int decimals[, 
                             string dec_point, string thousands_sep]]);
        返回格式化后的数字, 
        第一个参数, 要被格式化的数字; 
        第二个参数, 规定使用多少个小数位;
        第三个参数, 规定用什么字符串作为小数点; 
        第四个参数, 规定用作千位分隔符的字符串;
    3) md5
        将一个字符串进行MD5算法加密, 返回一个32位的十六进制字符串;
        string md5(string str[, bool raw_output]);
        str => 要被处理的字符串
        raw_output => TRUE将返回一个16位二进制数
                      FALSE返回32位的十六进制字符串
    4) md5_file
        使用方式与md5类似, 对文件进行MD5加密
