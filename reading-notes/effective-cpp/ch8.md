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
> ### **个人理解**
    使用nothrow new只是能保证operator new不抛出异常;


## **50: 了解new和delete的合理替换时机** ##
    (Understand when it makes sense to replace new and delete.)
    1) 有许多理由需要写个自定的new和delete, 包括改善效能, 对heap运用错误进行
       调试, 搜集heap使用信息
> ### **替换new和delete的理由**
    检测运用上的错误;
    搜集动态分配内存的使用统计信息;
    为了增加分配和归还的速度;
    为了降低缺省内存管理器带来的空间额外开销;
    为了弥补缺省分配器中的非最佳齐位;
    为了将相关对象成簇集中;
    为了获得非传统的行为;



## **51: 编写new和delete时需固守常规** ##
    (Adhere to convention when writing new and delete.)
    1) operator new应该内含一个无穷循环, 并在其中尝试分配内存, 如果它无法满
       足内存需求, 就该调用new-handler; 它也应该有能力处理0bytes申请; 类专属
       版本则还应该处理"比正确大小更大的(错误)申请"
    2) operator delete应该在收到null指针时不做任何事, 类专属版本应该处理"比
       正确大小更大的(错误)申请"
> ### **个人理解**
    对于有继承情况的内存操作, 我们可以使用如下操作:
    void* Base::operator new(std::size_t size) throw(std::bad_alloc) {
      if (size != sizeof(Base))
        return ::operator new(size);
      ...
    }
    这样就实现了某个类所专属的内存分配函数;
    C++保证删除null指针永远安全;
    要同new一样创建某个类的专属delete函数如下:
    void Base::operator delete(void* ptr, std::size_t size) throw() {
      if (nullptr == ptr)
      return;
      if (size != sizeof(Base)) {
        ::operator delete(ptr);
        return;
      }
      ... // 其他操作
    }




## **52: 写了placement new也要写placement delete** ##
    (Write placement delete if you write placement new.)
    1) 如果operator new接受的参数除了一定会有的那个size_t之外还有其他, 这个
       就是placement new
    2) 类似于new的placement版本, operator delete如接受额外参数, 为placement
       delete
    3) placement delete只有在"伴随placement new调用而触发的构造函数"出现异常
       时才会被调用; 对一个指针施行delete绝不会导致调用placement delete
    4) 当你写一个placement operator new, 请确定也写出了对应的placement 
       operator delete; 如果没有这么做, 你的程序可能会发生隐微而时断时续的
       内存泄漏
    5) 当你声明placement new和placement delete, 请确定不要无意识地遮掩它们的
       正常版本
> ### **个人理解**
    如果自己实现了placement new和placement delete, 如果在使用new的时候类的构
    造函数发生了异常会自动调用placement delete, 而不会泄露内存;
