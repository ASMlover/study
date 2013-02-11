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
> ### **2.4 其他字符串格式化函数** ###
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




## **3. 字符串比较函数** ##
> ### **3.1 按字节顺序进行字符串比较** ###
        int strcmp(string s1, string s2);     => 区分字符大小写
        int strcasecmp(string s1, string s2); => 忽略字符大小写
        函数返回值:
           如果s1等于s2则返回0
           如果s1大于s2则返回1
           如果s1小于s2则返回-1
> ### **3.2 按自然排序进行字符串比较** ###
        就是按照人们的日常生活中的思维习惯进行排序, 即将字符串中的数字部分按
    照数字大小进行比较.
        int strnatcmp(string s1, string s2);




## **4. 正则表达式在字符串处理中的应用** ##
> ### **4.1 正则表达式语法规则** ###
        正则表达式是由原子(普通字符, 如a到z), 有特殊功能的字符(称为元字符, 
    如*, +和?等), 以及模式修正符三个部分组成的文字模式
    1) 定界符
        使用正则表达式需要将模式表达式放到定界符之间, 作为定界符的字符通常有
        "/", 以及除了字母, 数字和"\"之外的任何字符。
    2) 原子
        是正则表达式的最近本组成单位, 且在每个模式中最少要包含一个原子; 原子
        包括所有的大写和小写字母字符, 所有数字, 所有标点符号以及一些其他字符
        还包括一些非打印字符
            \cx   : 匹配由x指明的控制字符
            \f    : 匹配一个换页符
            \n    : 匹配一个还行符
            \r    : 匹配一个回车符
            \s    : 匹配一个空白符(包括空格, 制表符, 换页符等)
            \S    : 匹配任何非空白符
            \t    : 匹配一个制表符
            \v    : 匹配一个垂直制表符
    3) 元字符
        就是用于构建正则表达式的具有特殊含义的字符, 如果要在正则表达式中包含
        元字符本身, 就需要在其前面加上转义字符"\"
            \d    : 匹配任意一个十进制数
            \D    : 匹配任意一个除十进制数字外的字符
            \s    : 匹配任意一个空白符
            \S    : 匹配任意非空白符意外的一个字符
            \w    : 匹配任意一个数字, 字母或下划线
            \W    : 匹配数字, 字母或下划线以外的任意一个字符
            *     : 匹配0次, 1次或多次其前的原子
            +     : 匹配1次或多次其前的原子
            ?     : 匹配0次或1次或多次其前的原子
            .     : 匹配除换行符意外的任意一个字符
            |     : 匹配两个或多个分支选择
            {n}   : 表示其前面的原子恰好出现n次
            {n, } : 表示其前面的原子出现不少于n次
            {n,m} : 表示其前面的原子至少出现n次, 最多出现m次
            ^或\A : 匹配输入字符串的开始位置
            $或\Z : 匹配输入字符串的结束位置
            \b    : 匹配单词的边界
            \B    : 匹配除单词边界以外的部分
            []    : 匹配方括号中指定的任意一个原子
            [^]   : 匹配除方括号中的原子意外的任意一个字符
            ()    : 匹配其整体为一个原子(由多个单个原子组成的大原子)
        通用字符类型:
            通用字符类型可以匹配相应类型中的一个字符, 包括(\d, \D, \s, \S,
            \w以及\W)
        限定符:
            用来指定正则表达式的一个给定原子必须出现多少次才能满足匹配, 包
            括(*, +, ?, {n}, {n,}以及{n, m})
        边界限制:
            限定字符串或单词的边界范围, 以获得更准确的匹配结果;
            ^或\A => 指字符串的开始
            $或\Z => 指字符串的结束
            \b    => 描述字符串中每个单词的前或则后边界
            \B    => 表示非单词边界
        句号(.):
            圆点可以匹配目标中的任何一个字符, 包括不可打印字符, 不匹配换行符
        模式选择符(|):
            用来分隔多选一模式, 在正则表达式中匹配两个或更多的选择之一
        原子表([]):
            可以定义一组彼此地位平等的原子, 且从原子表中仅选择一个原子进行匹
            配。
        模式单元:
            使用元字符()将多个原子组成大的原子, 被当作一个单元独立使用
        后向引用:
            这个小节还没弄明白???? 稍后在更新
            //! FIXME
        模式匹配的优先级:
            通常相同优先级是从左到右运算, 优先级表如下(顺序从上到下):
            转义符号          => \
            模式单元和原子表  => (), (?:), (?=), []
            重复匹配          => *, +, ?, {n}, {n,}, {n,m}
            边界限制          => ^, $, \b, \B, \A, \Z
            模式选择          => |
        模式修正符:
            在正则表达式定界符之外使用
            i : 在和模式进行匹配时不区分大小写
            m : 将字符串视为多行
            s : 将字符串视为单行, 换行符作为普通字符看待
            x : 模式中空白忽略不计, 除非它已经被转义
            e : 只用在preg_replace中, 在替换字符串中对逆向引用做正常替换
            U : 贪婪模式, 最大限度匹配
            D : 模式中的美元元字符仅匹配目标字符串的结尾





## **5. 与Perl兼容的正则表达式函数** ##
> ### **5.1 字符串的匹配与查找** ###
    1) preg_match
        按照指定的正则表达式的模式, 对字符串进行搜索和匹配
        int preg_match(string pattern, string subject[, array matches]);
        pattern => 提供用户按正则表达式语法编写的模式
        subject => 指定一个字符串
        该函数就是在subject中搜索pattern匹配的内容;
    2) preg_match_all
        与preg_match类似, 不同在于preg_match在第一次匹配后就停止搜索了, 而函
        数preg_match_all则会一直搜索到指定字符串的结尾
        int preg_match_all(string pattern, 
                           string subject, array matches[, int flags]);
        该函数将所有可能匹配的结果放到第三个参数的数组中, 并返回匹配次数;
        flags的取值为:
          PREG_PATTERN_ORDER  => 返回的结果1为全部模式匹配的数组, 
                                 结果2为第一个括号中的字模式所匹配的字符串
                                 ... 依此类推
          PREG_SET_ORDER      => 返回第一个为第一组匹配数组, 第二个为第二组
                                 匹配数组
    3) preg_grep
        匹配数组的元素, 返回与正则表达式匹配的数组单元;
        array preg_grep(string pattern, array input);
    4) 字符串处理函数strstr, strpos, strrpos, substr
        查找一个字符串中是否包含某个子字符串, 使用strstr和strpos;
        简单从一个字符串中取出一段子字符串, 使用substr;
        strstr对大小写敏感, stristr则忽略大小写;
        strpos返回字符串在另一个字符串中第一次出现的位置, 如没, 返回false;
        strpos和strrpos都对大小写敏感, stripos和strripos忽略大小写;
> ### **5.2 字符串的替换** ###
    1) preg_replace
        可执行正则表达式的搜索和替换, 
        mixed preg_replace(mixed pattern, mixed replacement, 
                           mixed subject[, int limit]);
            在subject中搜索与pattern匹配的项, 并替换为replacement, 替换次数
        为limit(limit == -1, 则所有匹配项都会被替换)
    2) str_replace
        实现字符串的替换工作, 处理简单的字符串替换时效率更高;
        mixed str_replace(mixed search, mixed replace, 
                          mixed subject[, int& count]);
            在subject中搜索与search匹配的项, 替换为replace, 替换的次数返回到
        count之中;
> ### **5.3 字符串的分割和连接** ###
    1) preg_split
        按正则表达式的方法分割字符串, 
        array preg_split(string pattern, 
                         string subject[, int limit[, int flags]]);
        flags的选项为:
            PREG_SPLIT_NO_EMPTY: 只返回非空部分
            PREG_SPLIT_DELIM_CAPTURE: 定界符模式中的括号表达式也会被捕获返回
            PREG_SPLIT_OFFSET_CAPTURE: 对每个出现的匹配结果也返回其附属字符
                                       串的偏移量
    2) explode
        如果仅用某个特定的字符串进行分割, 则它不会调用正则表达式引擎, 更快
        array explode(string separator, string string[, int limit]);
            separator为空(""), 返回FALSE, separator所包含的值在string中找不
        到, 那么返回包含string单个元素的数组
    3) implode
        与explode相反, 将数组中的所有元素组合成一个字符串;
        string implode(string glue, array pieces);
