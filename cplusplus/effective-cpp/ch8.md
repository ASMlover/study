# **定制new和delete** #
***


## **49: 了解new-handler的行为** ##
    (Understand the behavior of the new-handler.)
    1) 一个设计良好的new-handler应该满足:
        * 让更多内存可被使用
        * 安装另一个new-handler
        * 卸除new-handler
        * 抛出bad_alloc(或派生自bad_alloc)的异常
        * 不返回
    2) set_new_handler允许客户指定一个函数, 在内存分配无法获得满足时被调用
    3) nothrow new是一个颇为局限的工具, 因为它只适用于内存分配; 后继的构造
       函数调用还是可能抛出异常


## **50: 了解new和delete的合理替换时机** ##
    (Understand when it makes sense to replace new and delete.)
    1) 有许多理由需要写个自定的new和delete, 包括改善效能, 对heap运用错误进行
       调试, 搜集heap使用信息



## **51: 编写new和delete时需固守常规** ##
    (Adhere to convention when writing new and delete.)
    1) operator new应该内含一个无穷循环, 并在其中尝试分配内存, 如果它无法满
       足内存需求, 就该调用new-handler; 它也应该有能力处理0bytes申请; 类专属
       版本则还应该处理"比正确大小更大的(错误)申请"
    2) operator delete应该在收到null指针时不做任何事, 类专属版本应该处理"比
       正确大小更大的(错误)申请"
