# **Mark Sweep Garbage Collector**
***

## **Summary**
  * 使用了简易allocator的垃圾回收
  * 根据不同的大小规则，预先分配一定数量的chunk，需要时直接从中取
