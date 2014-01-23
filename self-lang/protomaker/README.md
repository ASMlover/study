# **README for ProtoMaker** #
***


## **Overview** ##
 * Just as Google Protobuf. 
 * Making protocol into C++/Java/C#... implementation.


## **Keywords** ##
    default   enum      protocol
    define    message   type   


## **Types** ##
        ProtoMaker          C++
        char/byte        char/unsigned char
        int8/uint8        int8_t/uint8_t
        int16/uint16      int16_t/uint16_t
        int32/uint32      int32_t/uint32_t
        int64/uint64      int64_t/uint64_t
        real32/real64     float/double


## **Operators** ##
    = < . [ ]


## **Examples** ##
    define NAME_LEN = 36
    protocol Message {
      enum {
        kMessageBegin = 0
        kMessageLogin
        ...
        kMessageEnd
      }
      uint8 __protocol 
    }

    message LoginMsg < protocol {
      __protocol default = Message.kMessageLogin
    }

    message LoginMsgC2S < LoginMsg {
      ...
    }

    message LoginMsgS2C < LoginMsg {
      ...
    }
