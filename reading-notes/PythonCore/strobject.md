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
