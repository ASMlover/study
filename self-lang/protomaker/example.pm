# example for protomaker language


define ACCOUNT_LEN = 36 # default length for account
define NAME_LEN = 36    # default length for name 
define IP = 3.1415926

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
  char account[ACCOUNT_LEN]
}

type PlayerInfo {
  char    name[NAME_LEN]
  uint8   photo
  uint32  id 
  uint32  coins
  uint32  scores
}

message LoginMsgS2C < LoginMsg {
  PlayerInfo info
}
