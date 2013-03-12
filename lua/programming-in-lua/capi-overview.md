# **C API概述** #
***

## **1. 第一个示例** ##
    1) lua.h定义了lua提供的基础函数, 包括创建lua环境, 调用lua函数, 读写lua环
       境中全局变量, 以及注册lua调用的新函数; 所有内容都有一个lua_前缀;
    2) lauxlib.h定义了辅助库提供的函数, 所有定义都以luaL_开头; lua的所有标准
       库都写到了辅助库;
       辅助库没有直接访问lua内部, 都是用官方的基础API来完成所有工作;
    3) lua库中没有定义任何全局变量, 将所有的状态都保存在动态结果lua_State中
    4) luaL_newstate用于创建一个新环境(或状态)
    5) luaL_loadbuffer编译用户输入的每行内容, 没有错误, 返回0, 并向栈中压入
       编译后的程序块
    6) lua_pcall会将程序块从栈中弹出, 并在保护模式中运行它


## **2. 栈** ##
> ### **2.1 压入元素** ###
    1) 每种可以呈现在lua中的C类型, API都有一个对应的压入函数
        常量nil                   lua_pushnil
        双精度浮点数              lua_pushnumber
        整数                      lua_pushinteger
        布尔(C中的整数)           lua_pushboolean
        任意字符串(char*及长度)   lua_pushlstring
        零结尾的字符串            lua_pushstring
    2) lua中的字符串不是以零结尾的, 他们额可以包含任意二进制数据;
       它们必须同时保存一个显示的长度;
    3) 向栈中压入一个元素时, 应该确保栈中具有足够的空间
    4) 调用一个具有很多参数的函数, 这时就要调用lua_checkstack来检查栈中是否
       有足够的空间
> ### **2.2 查询元素** ###
    1) API使用索引来引用栈中的元素; 第一个压入栈中的元素索引是1, 第二个是2, 
       以此类推直到栈顶; -1表示栈顶元素(最后压入的元素), -2表示栈顶下面的元
       素, 以此类推
    2) lua_is*系列函数 int lua_is*(lua_State* L, int index);
       lua_isnumber     检查值是否能转换为数字类型
       lua_istring      检查值能否转换为字符串类型
       lua_istable      检查元素是否是table
    3) lua_to*系列函数从战中获取一个值
       int lua_toboolean(lua_State* L, int index);
       lua_Number lua_tonumber(lua_State* L, int index);
       lua_Integer lua_tointeger(lua_State* L, int index);
       const char* lua_tolstring(lua_State* L, int index, size_t* len);
       size_t lua_objlen(lua_State* L, int index);
       当指定元素不具有正确的类型, lua_toboolean,lua_tonumber,lua_tointeger,
       lua_objlen返回0,, 其他返回NULL
> ### **2.3 其他栈操作** ###
    1) lua_gettop返回栈中的元素个数, 即是栈顶元素的索引
       int lua_gettop(lua_State* L);
    2) lua_settop将栈顶设置为一个指定的位置(修改栈中元素的数量); 如果之前的
       栈顶比新设置的更高, 那高出来的这些元素会被丢弃, 反之会向栈中压入nil来
       补足大小; lua_settop(L, 0)能清空栈
       void lua_settop(lua_State* L, int index);
    3) lua_pushvalue会将指定索引上的值的副本压入栈
       void lua_pushvalue(lua_State* L, int index);
    4) lua_remove删除指定索引上的元素, 并将该位置之上的所有元素下移以填空缺
       void lua_remove(lua_State* L, int index);
    5) lua_insert会上移指定位置之上的所有元素以开辟一个槽的空间, 然后将栈顶
       元素移到该位置
       void lua_insert(lua_State* L, int index);
    6) lua_replace弹出栈顶的值, 并将该值设置到指定的索引上, 但不会移动任何
       东西
       void lua_replace(lua_State* L, int index);


## **3. C API中的错误处理** ##
    1) lua中所有的结构都是动态的, 会根据需要来增长或缩小
> ### **3.1 应用程序代码中的错误处理** ###
    1) 通常, 应用程序代码是以"无保护"模式运行的
    2) luaL_newstate, lua_load, lua_pcall, lua_close都是安全的; 如果发生内存
       分配错误, 其他大多数函数都会抛出异常
    3) 当发生了内存分配错误, 又不想结束程序时;
       一种是设置一个"紧急"函数, 让它不要将控制权返回给lua
       第二是让代码在"保护模式"下运行
> ### **3.2 库代码中的错误处理** ###
        为lua编写库函数的时候, 当一个C函数检测到一个错误时, 就应该调用函数
    lua_error; lua_error会清理lua中所有需要清理的东西, 然后跳转回发起执行的
    那个lua_pcall, 并附上一条错误信息
