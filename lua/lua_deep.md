# **Lua深入知识** #
***


## **1. 函数** ##
> ### **1.1 格式** ###
        function <function_name>(<function_args>)
          <stmt>
        end
> ### **1.2 参数** ###
    1) 函数可以传递单个参数或多个参数, 多个参数之间用逗号隔开
    2) 函数参数使用(...)代替参数列表, lua会创建一个局部名字为arg的table, arg
       保存了所有调用时传递的参数; 参数格式可以通过arg.n来访问
> ### **1.3 返回值** ###
    1) 首先是类似C/C++的返回单个返回值
    2) 可以返回多个返回值, 多个返回结果之间使用逗号隔开




## **2. 标准库** ##
> ### **2.1 assert(myValue)()** ###
    1) 让你像处理函数一样运行编译后的lua代码块, 传入指向编译后的代码的变量, 
       然后立刻被执行;
    2) loadstring和assert经常配合使用, loadstring将编写好的代码块存到字符串
       中, 然后在assert中执行
    3) assert提供了更好的错误提示功能
> ### **2.2 dofile(filename)** ###
    1) 载入并立即执行lua脚本文件
    2) 通常是使用它来载入定义函数的文件以备调用
    3) 还可以载入数据文件
> ### **2.3 math.floor()** ###
    1) 向下取整, 会舍去小数部分
    2) 如想4舍5入, 可以先给数字加上0.5再向下去整
> ### **2.4 math.random()** ###
    1) math.random()随机生成一个0 ~ 1之间的伪随机数
    2) math.random(min, max)随机生成一个min ~ max之间的数字
    3) 在程序开始的时候, 最好给随机数种子设定一个唯一的值, 这样可以得到更好
       的随机数===>math.randomseed(os.date("%d%H%M%S"))
> ### **2.5 math.min/math.max** ###
    1) 返回最大值和最小值
    2) 他们都接收若干数字类型的参数



## **3. 字符处理** ##
> ### **3.1 类型转换** ###
    1) tonumber把字符串转换为数字
    2) tostring把数字转换为字符串
> ### **3.2 string.char(n1, n2, ...)** ###
    根据ASCII编码返回传入参数对应的字符
> ### **3.3 string.len(myString)** ###
    返回字符串的长度
> ### **3.4 string.sub(myString, start, end)** ###
    1) 返回myString从start到end的子串
    2) end省略则返回从start到myString末尾的子串
> ### **3.5 string.format()** ###
    1) 格式化输出指定的字符串
    2) 格式符与C相似
> ### **3.6 string.find(sourceString, findString)** ###
    1) 在sourceString中查找第一个符合findString的位置
    2) 找到, 返回它的开始位置和结束位置
    3) 没找到返回nil
> ### **3.7 字符和格式** ###
    1) lua支持的格式化符号:
        .       所有字符
        %a      字母
        %c      控制符
        %d      数字
        %l      小写字母
        %p      标点符号
        %s      空格符号
        %u      大写字母
        %w      字母数字
        %x      十六进制数
        %z      用0表示的字符
    2) string.gsub(sourceString, pattern, replacementString)
       sourceString中满足pattern格式的字符会被替换成replacementString
    3) string.gfind(sourceString, pattern)
       遍历一个字符串, 查找到符合指定格式的字符串就返回该子串




## **4. table数据结构** ##
> ### **4.1 table.getn(myTable)** ###
    1) 返回myTable中元素个数
    2) 只有使用数组方式的table才能正确求到元素个数
> ### **4.2 table.insert(myTable, position, value)** ###
    1) 在table中添加一个新值
    2) position是个可选参数, 没设定会添加到table的尾巴上
    3) 如果指定了position则会将值插入到指定的位置上
> ### **4.3 table.remove(myTable, position)** ###
    1) 从指定table中删除并返回一个元素
    2) 没指定position则删除table最后一个元素
> ### **4.4 table引用** ###
    1) 不仅可以使用数字作为索引, 还可以使用其他的值作为索引
    2) 类似与字典
> ### **4.5 pairs/ipairs** ###
    1) pairs可以遍历函数中的每一个元素
    2) ipairs只会遍历下标从1开始的元素



## **5. I/O基础** ##
    1) 打开文件
       file_handler = io.open(filename, "w")
    2) 打开文件失败会返回nil
    3) 打开文件的模式具体可以察看lua官方文档
