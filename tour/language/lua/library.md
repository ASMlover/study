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
