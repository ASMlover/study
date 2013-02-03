# **数组和数据结构** #
*** 

## **1 数组分类** ##
> ### **1.1 索引数组** ###
    1) 索引数组的索引值是数组
    2) 以0开始, 一次递增
> ### **1.2 关联数组** ###
    1) 关联数组以字符串为索引值
    2) 类似与C++中的Map



## **2. 数组的遍历** ##
> ### **2.1 使用for** ### 
    1) 同其他大部分语言一样
    2) 使用count函数来计算array的元素个数
> ### **2.2 foreach** ###
    1) 格式1
       foreach (array_expression as $value) {}
    2) 格式2
       foreach (array_expression as $key => $value) {}
    3) 格式1将数组每一项的值依次遍历
    4) 格式2将数组每一项的键和值依次遍历
> ### **2.3 联合使用list,each和while遍历数组** ###
>> #### **2.3.1 each** ####
    1) each返回数组当前元素的键值对, 以数组方式呈现
    2) 键名0, key对应当前元素的键名
    3) 键名1, value对应当前元素的值
    4) each返回NULL, 则到了数组的末端
>> #### **2.3.2 list** ####
    1) list只能用于数字索引的数组, 并假定索引从0开始
    2) list(mixed varname, mixed varname, ...) = array_expression
>> #### **2.3.3 while** ####
    1) while (list($key, $value) = each(array_expression)) {}

> ### **2.4 使用数组内部控制函数遍历** ###
    1) current => 取得目前指针位置的内容资料
    2) key => 读取目前指针所指资料的索引值
    3) next => 将数组的内部指针移动到下一个单元
    4) prev => 将数组的内部指针移动到上一个单元
    5) end => 将数组指针指向最后一个元素
    6) reset => 将指针移动到第一个元素



## **3. 预定义数组** ##
> 他们很特别, 在全局范围内生效, 因此也称为自动全局变量或超全局变量  
1) $_SERVER : 由web服务器设定或直接与当前脚本的执行环境关联  
2) $_ENV : 执行环境提交至脚本的变量  
3) $_GET : 经由URL请求提交到脚本的变量  
4) $_POST : 经由HTTP POST方法提交到脚本的变量  
5) $_REQUEST : 经由GET, POST, COOKIE提交到脚本的变量, 因此该数组不值得信任  
6) $_FILES : 经由HTTP POST文件上传而提交到脚本的变量  
7) $_COOKIE : 经由HTTP COOKIES方法提交到脚本的变量  
8) $_SESSION : 当前注册给脚本会话的变量  
9) $GLOBALS : 包含一个引用指向每一个当前脚本的全局范围内的有效变量, 该数组的键名为全局变量的名称  

> ### **3.1 服务器变量: $_SERVER** ###
    1) 是一个包含诸如头信息, 路径和脚本位置的数组
    2) 在函数或对象中不需要使用global就可以直接访问
> ### **3.2 环境变量: $_ENV** ###
    1) 在PHP解析器运行是, 从PHP所在的服务器中的环境变量转化为PHP全局变量
    2) 在所有脚本中都有效, 不需要使用global直接访问
> ### **3.3 HTTP GET变量: $_GET** ###
    1) 在服务器页面中通过$_GET获取URL或表单GET方式传递过来的参数
> ### **3.4 HTTP POST变量: $_POST** ###
    1) $_POST数组只能访问以POST方法提交的表单数据
> ### **3.5 request变量: $_REQUEST** ###
    1) 包含$_GET, $_POST, $_COOKIE中的全部内容
    2) 不管是POST还是GET方法提交的数据的都可以使用$_REQUEST获取
    3) $_REQUEST速度比较慢, 不推荐使用
> ### **3.6 HTTP文件上传变量: $_FILES** ###
    1) 必须使用POST提交
    2) $_FILES超全局数组是表单通过POST方法传递的已上传文件项目组成的数组
> ### **3.7 HTTP Cookies: $_COOKIE** ###
    1) 由HTTP Cookies方法提交到脚本的变量
    2) PHP脚本从客户浏览器取了一个cookie后, 将自动把他转换成一个变量, 可以
       通过$_COOKIE和cookie的名称来存取制定的cookie值
> ### **3.8 Session变量: $_SESSION** ###
    1) 会话控制是在服务端使用session跟踪用户
    2) 在服务器页面使用session_start函数开启session后, 就可使用$_SESSION注册
       全局变量, 用户可在整个网站中访问这些会话信息
> ### **3.9 Global变量: $GLOBALS** ###
    1) 在函数中使用函数外声明的全局变量是, 可以使用$GLOBALS代替global关键字
    2) $GLOBALS数组中, 每个变量是一个元素, 键名对应变量名




## **4. 数组的相关处理函数** ##
> ### **4.1 键/值操作函数** ###
    1) array_values
       返回数组中所有元素的值, 不保留键名, 返回的数组将使用顺序数组键重新建
       立索引, 从0递增
    2) array_keys
       返回数组中所有键名
       array array_keys(array input[, mixed search_value[, bool strict]]);
       指定search_value, 只返回指定改值的键名
       strict=false, 则指定的search_value以来类型; 否则据类型返回指定值键名
    3) in_array
       检查数组中是否存在某个值, 即在数组中搜索给定的值
       bool in_array(mixed needle, array haystack[, bool strict]);
    4) array_flip
       交换数组中的键和值, 如果同一个值出现多次, 则最后一个键名就是它的值
    5) array_reverse
       将原数组中的元素顺序翻转, 返回一个新的数组
