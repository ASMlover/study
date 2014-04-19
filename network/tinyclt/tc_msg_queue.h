// Copyright (c) 2014 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifndef __TINYCLT_MESSAGE_QUEUE_HEADER_H__
#define __TINYCLT_MESSAGE_QUEUE_HEADER_H__

struct NetMsg {
  enum MsgType {
    MSGTYPE_ERR = -1, 
    MSGTYPE_LINK, 
    MSGTYPE_BREAK, 
    MSGTYPE_DATA, 
  };

  uint32_t type;
  uint32_t size;
  char*    buffer;

  NetMsg(void) 
    : type(MSGTYPE_ERR) 
    , size(0) 
    , buffer(nullptr) {
  }

  void Reset(void) {
    if (MSGTYPE_DATA == type && nullptr != buffer) {
      delete [] buffer;
      buffer = nullptr;
    }
    type = MSGTYPE_ERR;
    size = 0;
  }
};


class MsgQueue : private UnCopyable {
  SpinLock locker_;
  std::queue<NetMsg> msg_queue_;
public:
  MsgQueue(void);
  ~MsgQueue(void);

  void Push(const NetMsg& msg);
  bool Pop(NetMsg& msg);

  void Clear(void);
};


#endif  // __TINYCLT_MESSAGE_QUEUE_HEADER_H__
