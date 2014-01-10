# **README for ProtoMaker** #
***


## **Overview** ##
 * Just as Google Protobuf. 
 * Making protocol into C++/Java/C#... implementation.


## **Keywords** ##
    default   message     type
    enum      protocol   


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


## **Operators** ##
    = < .


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

    message LoginMsg < protocol {
      __protocol default = Message.kMessageLogin
    }

    message LoginMsgC2S < LoginMsg {
      ...
    }

    message LoginMsgS2C < LoginMsg {
      ...
    }
