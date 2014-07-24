# **库**
***



## **1. 数学库**
    1) 三角函数
        sin, cos, tan, asin, acos
        deg, rad转换角度和弧度
    2) 指数对数函数
        exp, log, log10
    3) 取整函数
        floor, ceil, max, min
    4) 伪随机函数
        random, randomseed
    5) 变量pi和huge(lua可表示的最大数字)



## **2. table库**
    1) 插入和删除
        * table.insert将一个元素插入到一个数组的指定位置, 默认插入到数组尾;
        * table.remove删除并返回数组指定位置上的元素, 并前移之后的元素;
    2) 排序
        * table.sort对一个数组排序, 还可以指定一个次序函数; 次序函数有两个
          参数, 如果希望第一个参数排在第二个之前, 应返回true; sort默认使用
          小于操作;
    3) 连接
        * table.concat接受一个字符串数组, 并返回这些字符串连接后的结果;
          concat的扩展, 可处理嵌套数组
              function reconcat(arr)
                if type(arr) ~= 'table' then
                  return arr
                end
                local res = {}
                for i=1, #arr do
                  res[i] = reconcat(arr[i])
                end
                return table.concat(res)
              end



## **3. 字符串库**
> ### **3.1 基础字符串函数**
    1) string.len(s) 返回字符串s的长度
    2) string.rep(s, n) 返回字符串s重复n次的结果
    3) string.lower(s) 返回一份s副本, 所有大写字母转换为小写形式
    4) string.upper(s) 返回一份s副本, 所有小写字母转换为大写形式
    5) string.sub(s, i, j) 从字符串中取第i个到第j个字符;
       1表示第一个字符, -1表示第二个字符;
    6) string.char/string.byte转换字符以及内部数值表示
    7) string.format类似printf

> ### **3.2 模式匹配函数**
    1) string.find函数
       用于在一个给定的目标字符串中搜索一个模式, 返回匹配到的起始索引和结尾
       索引; 没找到任何匹配就返回nil;
       匹配成功可以用返回值来调用string.sub提取出目标字符串中匹配于该模式的
       那部分子串;
    2) string.match函数
       与string.find类似, 不过返回与模式匹配的子串;
    3) string.gsub函数
       有3个参数: 目标字符串, 模式和替换字符串; 将目标字符串中所有出现模式的
       地方替换为替换字符串;
    4) string.gmatch函数
       返回一个函数, 通过这个函数可以遍历到一个字符串中所有出现指定模式的地
       方;

> ### **3.3 模式**
    1) 字符分类
        .   -> 所有字符
        %a  -> 字母
        %c  -> 控制字符
        %d  -> 数字
        %l  -> 小写字母
        %p  -> 标点符号
        %s  -> 空白字符
        %u  -> 大写字母
        %w  -> 字母和数字字符
        %x  -> 十六进制数字
        %z  -> 内部表示为0的字符
    2) 在一个字符集前面加上^表示该字符集的补集; [^\n]表示除换行之外的其他字
       符; 对简单的分类, 使用其大写字母可以得到其补集;
    3) 模式的4中修饰符:
        +   -> 重复1次或多次
        *   -> 重复0次或多次
        -   -> 重复0次或多次
        ?   -> 可选部分(出现0次或1次)

> ### **3.4 捕获**
    1) 根据一个模式从目标字符串中抽出匹配于该模式的内容; 在指定捕获时应该将
       模式中需要捕获的部分写到一对圆括号内;
       对于具有捕获的模式, string.match会将所有捕获到的值作为单独的结果返回;
    2) 对于捕获到的值, 还可以用于gsub函数的字符串替换掉;

> ### **3.5 替换**
    1) string.gsub第3个参数不仅是一个字符串, 还可以是一个函数或者table; gsub
       会用每次捕获到的内容作为key在table中进行查找, 并将对应的value作为要替
       换的字符串; 如果table中没有对应的key, 那么string.gsub不改变这个匹配;

> ### **3.6 技巧**
    1) 在模式开始或结束处使用修饰符'-'是没有意义的, 因为总是匹配到空字符串;




## **4. IO库**
> ### **4.1 简单IO模型**
    1) 当前输入文件为标准输入stdin, 当前输出文件为标准输出stdout;
    2) io.input和io.output可以改变这两个当前文件;
    3) io.write接受任意数量的字符串参数并将它们写入当前输出文件;
    4) 原则: 在随意编写的程序或为调试目的提倡使用print; 而在其他需要控制输出
       的地方使用write;
    5) io.read从当前输入文件中读取字符串, 参数取决定要读取的数据:
        '*all'    -> 读取整个文件
        '*line'   -> 读取下一行
        '*number' -> 读取一个数字
        <num>     -> 读取一个不超过<num>个字符的字符串
       io.read('*all')是从当前位置读取当前输入文件的所有内容;
    6) io.lines迭代器可以迭代文件中的所有行;
    7) io.read(0)是一种特殊情况, 用于检查是否达到了文件的末尾; 如果还有数据
       可读, 返回一个空字符串, 否则返回nil;

> ### **4.2 完整IO模型**
    1) 基于文件句柄, 等价于C语言中的流(FILE*)
    2) io.open打开文件, 类似fopen; 'r'表示读取, 'w'表示写入, 'a'表示追加, 
       'b'表示打开二进制文件; 
       io.open返回表示文件的句柄, 若发生错误, 则返回nil和一条错误信息和一个
       错误码;
    3) 打开文件成功后就可以使用read/write来读写文件了;
    4) IO库提供3个预定义C语言流的局部: io.stdin, io.stdout, io.stderr;
    5) 为了提高效率可以一次读取一个块, 同时为了避免在行中间断开, 只需要在读
       一个块的时候加上一行:
          local lines, rest = f:read(BUFFSIZ, '*line')
    6) io.flush会刷新当前输出文件; f:flush会刷新某个特定的文件;
    7) f:seek(whence, offset)可以获取和设置一个文件的当前位置, whence是一个
       字符串, 指定如何解释offset参数:
       * 'set'  -> offset解释为相对文件起始的偏移量;
       * 'cur'  -> offset解释为相对于当前位置的偏移量;
       * 'end'  -> offset解释为相对于文件末尾的偏移量;
       该函数总是返回文件的当前位置;



## **5. 操作系统库**
> ### **5.1 日期和时间**
    1) time和date提供了所有的日期和时间功能;
    2) 不带任何参数的time会以数字形式返回当前的日期和时间;
    3) 以table作为参数调用, 会返回一个数字表示该table中所描述的日期和时间;
       table具有以下字段:
          year    -> 一个完整的年份
          month   -> 01 ~ 12
          day     -> 01 ~ 31
          hour    -> 00 ~ 23
          min     -> 00 ~ 59
          sec     -> 00 ~ 59
          isdst   -> 一个布尔值表示夏令时
       year, month, day是必须的; 其他字段默认为12:00:00;
    4) os.date会将日期格式化为一个字符串;
        %a    -> 一星期中天数的简写(例如: Wed)
        %A    -> 一星期中天数的全称(例如: Wednesday)
        %b    -> 月份的简写(例如: Sep)
        %B    -> 月份的全称(如: September)
        %c    -> 日期和时间(如: 09/16/98 23:48:10)
        %d    -> 一个月中的第几天(16)[01 ~ 31]
        %H    -> 24小时制中的小时数(23)[00 ~ 23]
        %I    -> 12小时制中的小时数(11)[01 ~ 12]
        %j    -> 一年中的第几天(259)[001 ~ 366]
        %M    -> 分钟数(48)[00 ~ 59]
        %m    -> 月份数(09)[01 ~ 12]
        %p    -> 上午(am)或下午(pm)
        %S    -> 秒数(10)[00 ~ 59]
        %w    -> 一星期中的第几天(3)[0 ~ 6 = 星期天 ~ 星期六]
        %x    -> 日期(09/16/98)
        %X    -> 时间(23:48:10)
        %y    -> 两位数的年份(98)[00 ~ 99]
        %Y    -> 完整的年份(1998)
        %%    -> 字符'%'
    5) os.clock返回当前CPU的时间秒数

> ### **5.2 其他系统调用**
    1) os.exit中止当前程序的执行;
    2) os.getenv可获取一个环境变量的值, 并接受一个变量, 返回对应的字符串值;
    3) os.execute可运行一条系统命令;
    4) os.setlocale设置当前Lua程序所使用的区域; 区域中的6种分类:
        * 'collate'控制字符串的字母顺序;
        * 'ctype'控制单个字符类型以及其大小写间的转换;
        * 'monetary'不影响Lua程序;
        * 'numeric'控制如何格式化数字;
        * 'time'控制如何格式化日期和时间;
        * 'all'控制上述所有功能;



## **6. 调试库**
    由两类组成:
      * 自省函数: 允许检查一个正在运行中程序的各方面;
      * 钩子: 允许跟踪一个程序的执行;

> ### **6.1 自省机制**
    1) debug.getinfo 
        * 第一个参数是一个函数或一个栈层;
        * 到为某函数foo调用debug.getinfo(foo)会得到一个table, 包含了一些与该
          函数相关的信息;
    2) debug.getinfo返回的table有如下字段:
        * source: 函数定义的位置
        * short_src: source的短版本, 可用于错误信息中
        * linedefined: 该函数定义在源代码中第一行的行号
        * lastlinedefined: 该函数定义在源代码中最后一行的行号
        * what: 函数的类型
        * name: 该函数的一个适当的名称
        * namewhat: 上一个字段的含义, 可能是'global', 'local', 'method', 
          'field'或''
        * nups: 该函数的upvalue的数量
        * activelines: 一个table, 包含了该函数的所有活动行的集合
        * func: 函数本身
    3) debug.getinfo的第二个参数用于指定希望获取哪些信息, 可选值如下:
        * 'n' -> 选择name或namewhat
        * 'f' -> 选择func
        * 'S' -> 选择source, short_src, what, linedefined和lastlinedefined
        * 'l' -> 选择currentline
        * 'L' -> 选择activelines
        * 'u' -> 选择nups

> ### **6.2 访问局部变量**
    1) debug.getlocal用于检查任意活动函数的局部变量
    2) 有两个参数: 
        * 希望查询的函数栈层
        * 变量的索引
       返回两个值变量的名字和它的当前值, 如果变量索引大于活动变量的总数, 
       那么getlocal返回nil; 如果栈层无效, 引发错误;

> ### **6.3 访问非局部的变量**
    1) getupvalue可以访问一个Lua函数所使用的非局部变量;
    2) getupvalue的第一个参数不是栈层而是一个函数(closure), 第二个参数是变量
       索引;
    3) 可以使用debug.setupvalue来修改非局部变量, 有3个参数:
        * closure
        * 一个变量索引
        * 一个新值
       返回变量的名称, 如果变量索引超出范围, 则返回nil;

> ### **6.4 钩子**
    1) 触发一个钩子的4种事件:
        * 当Lua调用一个函数时产生的call事件;
        * 当函数返回时产生的return事件;
        * 当Lua开始执行一行新代码时产生的line事件;
        * 当执行完指定数量的指令后产生的count事件;
    2) 注册一个钩子, 需要为debug.sethook设置2个或3个参数:
        * 钩子函数
        * 一个字符串, 描述需要监控的事件
        * 一个可选数字, 用于说明多久获得一次count事件
    3) 需要监控call, return和line事件, 需要将他们的首字母(c, r, l)放入到掩码
       字符串;

> ### **6.5 性能剖析**
    1) 如果做计时剖析, 最好使用C接口;
    2) 但对于计数性剖析, 钩子还是很好用的:
          local function Hook()
            local f = debug.getinfo(2, 'f').func 
            if counters[f] == nil then
              counters[f] = 1
              names[f] = debug.getinfo(2, 'Sn')
            else
              counters[f] = counters[f] + 1
            end
          end
        运行profiler:
          $ lua profiler xxx.lua
