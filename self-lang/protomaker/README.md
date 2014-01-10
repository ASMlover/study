# **README for ProtoMaker** #
***


## **Overview** ##
 * Just as Google Protobuf. 
 * Making protocol into C++/Java/C#... implementation.


## **Keywords** ##
    default   extend    protocol
    enum      message   type


## **Types** ##
      ProtoMaker          C++
        byte              char
        int8              int8_t
        uint8             uint8_t
        int16             int16_t
        uint16            uint16_t
        int32             int32_t
        uint32            uint32_t
        int64             int64_t
        uint64            uint64_t
        real32            float
        real64            double


## **Examples** ##
    protocol Message {
      enum {
        kMessageBegin = 0
        kMessageLogin
        ...
        kMessageEnd
      }
      uint8 __protocol 
    }

    message LoginMsg extend protocol {
      __protocol default = kMessageLogin
    }

    message LoginMsgC2S extend LoginMsg {
      ...
    }

    message LoginMsgS2C extend LoginMsg {
      ...
    }
