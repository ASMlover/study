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
