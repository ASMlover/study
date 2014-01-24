# **README for ProtoMaker** #
***


## **Overview** ##
 * Just as Google Protobuf. 
 * Making protocol into C++/Java/C#... implementation.


## **Keywords** ##
    default   enum      protocol
    define    message   type   


## **Types** ##
        ProtoMaker          C++                       Java
        char/byte        char/unsigned char           byte
        int8/uint8        int8_t/uint8_t              byte
        int16/uint16      int16_t/uint16_t            short
        int32/uint32      int32_t/uint32_t            int
        int64/uint64      int64_t/uint64_t            long
        real32/real64     float/double                float/double


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
