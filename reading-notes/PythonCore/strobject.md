# **Python中的字符串对象**
***

## **1、PyStringObject**
不同的PyStringObject对象，其内部所需的保存字符串内容的内存空间是不一样的，PyStringObject是一个不可变对象，创建一个PyStringObject对象后，该对象内部维护的字符串就不能再改变了；PyStringObject对象定义如下：
```C++
class PyStringObject : public PyVarObject {
  long ob_shash; // 缓存该对象的hash值，避免每一次都重新计算该字符串对象的hash值；
                 // 如果该对象还没被计算过hash值，那其初始值是-1
  int ob_sstate;
  char ob_sval[1];
};

// 计算PyStringObject的hash值算法如下：
static long string_hash(PyStringObject* s) {
  if (s->ob_shash != -1)
    return s->ob_shash;
  auto len = s->ob_size;
  auto* p = reinterpret_cast<unsigned char*>(s->ob_sval);
  long x = *p << 7;
  while (--len >= 0)
    x = (1000003 * x) ^ *p++;
  x ^= s->ob_size;
  if (x == -1)
    x = -2;
  s->ob_shash = x;
  return x;
}
```
PyStringObject内部维护的字符串在末尾以'\0'结尾，但实际长度是由`ob_size`维护，所以PyStringObject表示的字符串对象中间可能也出现'\0'，所以指向的是一段长度为`ob_size+1`个字节的内存；

## **2、创建PyStringObject**
在`PyString_FromString`中创建对象，如果传入的字符串为空串则传回nullstring指向的对象，在第一次创建nullstring的时候会通过intern机制进行共享；如果不是空串则申请内存创建对象，将hash缓存位-1，intern表示设置为`SSTATE_NOT_INTERNED`，将str指向的字符数组拷贝到PyStringObject所维护的空间；

## **3、intern机制**
PyStringObject引入intern机制，对某个string对象应用了intern机制，之后再创建相同的字符串对象的时候会先在系统中记录的已经被intern机制处理了的PyStringObject对象中查找，如果已经存在则将该对象的引用返回，不用再重新创建一个PyStringObject对象；

## **4、字符缓冲池**
PyObjectString中的一个字节的字符对应的PyStringObject对象保存在characters中，其大小位`UCHAR_MAX(0xff)`，当字符串对象为一个字符时先对所创建的字符串对象进行intern操作然后将intern的结果缓存到characters中；当然在创建string对象的时候会先判断其是否只有一个字符，然后在缓冲池中查找，如果找到则直接返回；

## **5、效率相关**
通过'+'来拼接字符串的方式效率是很低下的，是由于string是不可变对象，这样拼接的时候必须创建一个新的string对象，这样N个string对象拼接则需要进行N-1次内存申请和内存拷贝工作；所以应该使用join来对存储在list或tuple中的一组string对象进行拼接操作，这样只需要分配一次内存；
