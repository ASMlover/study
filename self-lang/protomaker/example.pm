# example for protomaker language

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
  real32 coins
}

message LoginMsgC2S < LoginMsg {
  coins default = 100.15
  byte account[36]
}

message LoginMsgS2C < LoginMsg {
  byte   name[36]
  uint8  photo
  uint32 player_id
  uint32 coins
  uint32 scores
}
