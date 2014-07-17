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
