protocol Message {
  enum {
    kMessageBegin = 0
    kMessageLogin
    kMessageLogout
    kMessageEnd
  }
  uint8 __protocol 
}

message LoginMsg < protocol {
  __protocol default = Message.kMessageLogin
}

message LoginMsgC2S < LoginMsg {
}

message LoginMsgS2C < LoginMsg {
  uint32 id
}
