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
    它们很特别, 在全局范围内生效, 因此也称为自动全局变量或超全局变量  
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
> ### **4.2 统计数组元素的个数和唯一性** ###
    1) count
       计算数组中的元素数目或对象中的属性个数, 对于数组, 返回元素个数; 其他
       返回1。
    2) array_count_values
       统计数组中所有值出现的次数, 其返回是一个数组, 键是原数组的值, 值是原
       数组中该值出现的次数
       array array_count_values(array input);
    3) array_unique
       用于删除数组中重复的值, 并返回没有重复值的新数组
> ### **4.3 使用回调函数处理数组的函数** ###
    1) array_filter
       用回调函数过滤数组中的元素, 返回按用户自定义函数过滤后的新数组
       array array_filter(array input[, callback callback]);
       <function callback($value)>
    2) array_walk
       对数组中的每个元素应用回调函数处理
       bool array_walk(array& array, callback funcname[, mixed userdata]);
       <function callback($value, $key)>
    3) array_map
       相比array_walk更加灵活, 可以处理多个数组, 将回调函数作用到给定数组的
       元素上, 返回用户自定义函数作用后的数组
       array array_map(callback callback, array array1[, array ...]);
       <function callback($value)>
> ### **4.4 数组的排序函数** ###
    1) 简单的数组排序函数
       对一个数组元素的值进行排序, 键将被忽略, 使用0 ~ n的下标
       bool sort(array& array[, int sort_flags]);   => 由小到大
       bool rsort(array& array[, int sort_flags]);  => 由大到小
       sort_flags可选值
          SORT_REGULAR  => 默认值, 自动识别数组元素的类型进行排序
          SORT_NUMERIC  => 数字元素排序
          SORT_STRING   => 字符串元素排序
          SORT_LOCALE_STRING => 根据当前locale设置把元素当作字符串比较
    2) 根据键名对数组排序
       bool ksort(array& array[, int sort_flags]);  => 由小到大
       bool krsort(array& array[, int sort_flags]); => 由大到小
       sort_flags与sort/rsort相同
    3) 根据元素的值对数组排序
       根据元素的值对数组排序, 结果将保留原有键名和值对应的关系;
       bool asort(array& array[, int sort_flags]);
       bool arsort(array& array[, int sort_flags]);
    4) 根据"自然排序"法对数组排序
       不是使用计算规则, 数字从1到9, 字母a到z, 短者优先。
       bool natsort(array& array);     => 结果忽略键名
       bool netcasesort(array& array); => 对数组不区分大小写字母的排序
    5) 根据用户自定的规则对数组排序
       bool usort(array& array, callback cmp_function);
       bool uasort(array& array, callback cmp_function);
       bool uksort(array& array, callback cmp_function);
       这个几个函数与sort, asort, ksort对应 
    6) 多维数组的排序
       bool array_multisort(array a1[, mixed arg[, mixed ...[, array ...]]])
       具体请参见PHP手册
> ### **4.5 拆分/合并/分解/接合数组** ###
    1) array_slice
       在数组中根据条件取出一段值并返回
       array array_slice(array array, int offset[, int len[, bool preserve_keys]]);
       offset => 规定元素的开始位置
       len    => 规定返回数组的长度
       preserve_keys => 规定是否保留键名
    2) array_splice
       与array_slice相似, 选择数组中的一些列元素, 删除并用其他值替换
       array_splice(array& input, int offset[, int len[, array replacement]]);
       len  => 表示从offset开始的len个元素
       replacement => 表示替换的元素数组
    3) array_combine
       合并两个数组, 一个数组是键名, 另一个是键值, 如果一个数组为空, 或者两
       个数组的个数不同返回false
       array array_combine(array keys, array values);
    4) array_merge
       将一个或多个数组合并为一个数组, 如果键重复, 则后面的覆盖前面的;
       如果传入函数的只有一个数组, 且键是数字, 则返回的键名以0开始
       array array_merge(array a1[, array a2[, array ...]]);
    5) array_intersect
       计算数组的交集, 键名保持不变, 仅由值进行比较
       array array_intersect(array a1, array a2[, array ...]);
    6) array_diff
       返回数组的差集, 键名保持不变, 仅由值进行比较(返回1有2没有的元素)
       array array_diff(array a1, array a2[, array ...]);
> ### **4.6 数组与数据结构** ###
    1) 使用数组实现堆栈
       array_push向第一个数组尾巴上添加一个元素, 返回新数组的长度:
          int array_push(array& array, mixed var[, mixed ...]);
       array_pop删除数组中的最后一个元素, 返回弹出的元素值:
          mixed array_pop(array& array);
    2) 使用数组实现队列
       array_push向数组尾添加一个元素;
       array_shift删除数组的第一个元素, 并返回被删除元素的值:
          mixed array_shift(array& array);
> ### **4.7 其他有用的数组处理函数** ###
    1) array_rand
       从数组中随机选出一个或多个元素, 并返回 
          mixed array_rand(array input[, int num_req]);
          第一个参数是一个输入数组; 
          第二个参数是指定你想取出多少个元素。
    2) shuffle
       将数组中的元素按随机顺序重新排列, 成功返回TRUE, 否则返回FALSE 
    3) array_sum
       返回数组中所有值的总和, 如果所有值都是整数, 返回一个整数值, 如果其中
       有一个或多个值是浮点数, 则返回浮点数
    4) range
       创建并返回一个包含指定范围的元素的数组
          array range(mixed first, mixed second[, number step]);
          第一个参数 => 规定数组元素的最小值
          第二个参数 => 规定数组元素的最大值
          第三个参数 => 规定元素之间的步进制
