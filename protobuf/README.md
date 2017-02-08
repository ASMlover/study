# **README for Protobuf**
***

## **Summary**
 * 一些Protobuf的使用技巧以及对Protobuf的理解

## **Building**
 * 从github下载后根据cmake/README.md来编译
``` bash
  $ git clone https://github.com/google/protobuf.git
  $ cd protobuf/cmake
  $ mkdir cmake-build
  $ cd cmake-build
  $ cmake ..
  $ make
```

## **1. Protobuf的数据类型**
    enum WriteType {
      WRITETYPE_VARINT            = 0,
      WRITETYPE_FIXED64           = 1,
      WRITETYPE_LENGTH_DELIMITED  = 2,
      WRITETYPE_START_GROUP       = 3,
      WRITETYPE_END_GROUP         = 4,
      WRITETYPE_FIXED32           = 5,
    };
    VARINT:           int32,int64,uint32,uint64,sint32,sint64,bool,enum
    FIXED64:          fixed64,sfixed64,double
    LENGTH_DELIMITED: string,bytes,embedded messages,packed repeated fields
    START_GROUP:      groups(deprecated)
    END_GROUP:        groups(deprecated)
    FIXED32:          fixed32,sfixed32,float

## **2. 对Protobuf的理解**
 * 是一种编码格式
 * 是一种tlv的实现
  - t是tag, 指(id,type)
  - l是length, 是可选参数, 根据type有时是固定的, 此时length可省略
  - 内部就是(id, type+[length], value)的组合

## **3. Message的格式**
    |key|Value|key|Value| ... |key|Value|
    Key的定义如下:
      key = (field_number << 3) | write_type
    在解析数据包的时候, 根据key就可以知道Value对应消息中的哪个field了;
    Message编码后, key-value被编码成字节流存储; 在解码的时候, 会跳过不能识别
    的字段, 所以即使增加了新字段, 也不会影响老的程序, 因为老程序根本就不识别
    这些新增加的字段;
