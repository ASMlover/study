# **README for Protobuf**
***


## **Summary**
 * 一些Protobuf的使用技巧以及对Protobuf的理解



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
