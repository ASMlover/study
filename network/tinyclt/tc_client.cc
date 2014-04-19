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
#include "tc_header.h"
#include "tc_socket.h"
#include "tc_msg_queue.h"
#include "tc_client.h"


struct MsgHead {
  uint16_t bytes;
  uint16_t sequence;
};

#define BUFFER_LEN  (0x20000)
static char s_rbuffer[BUFFER_LEN];


Client::Client(void)
  : connected_(false) 
  , wsequence_(0) 
  , client_(new Socket()) 
  , thread_(new Thread()) 
  , msg_queue_(new MsgQueue()) {
}

Client::~Client(void) {
}


bool Client::Connect(const char* address, uint16_t port) {
  if (connected_) 
    Disconnect();

  if (!client_->Open())
    return false;
  if (!client_->Connect(address, port))
    return false;
  connected_ = true;

  thread_->Create(THREAD_CALLBACK(Client::ReadRoutine, this));

  return true;
}

void Client::Disconnect(void) {
  connected_ = false;

  if (nullptr != client_.get())
    client_->Close();

  if (nullptr != thread_.get())
    thread_->Join();
}

bool Client::Dispatch(DispatcherType dispatcher, uint32_t timeout) {
  if (nullptr == dispatcher)
    return false;

  uint32_t beg = GetCurrentMS();
  uint32_t end;

  NetMsg msg;
  while (msg_queue_->Pop(msg)) {
    dispatcher(msg.type, msg.buffer, msg.size);

    msg.Reset();

    end = GetCurrentMS();
    if (end - beg >= timeout)
      break;
  }

  return true;
}

bool Client::Write(const void* buffer, uint32_t bytes) {
  if (!connected_)
    return false;

  MsgHead head;
  head.sequence = wsequence_;
  head.bytes = (uint16_t)bytes;

  int ret = client_->Write((const char*)&head, sizeof(head));
  if (NETTYPE_ERR == ret)
    return false;

  ret = client_->Write((const char*)buffer, (int)bytes);
  if (NETTYPE_ERR == ret)
    return false;

  ++wsequence_;
  return true;
}

void Client::ReadRoutine(void* argument) {
  uint32_t rsequence = 0;
  while (connected_) {
    MsgHead head;
    if (NETTYPE_ERR == client_->Read(sizeof(head), (char*)&head)) 
      break;

    if (head.sequence != (uint16_t)rsequence)
      break;

    if (NETTYPE_ERR == client_->Read(head.bytes, s_rbuffer))
      break;

    NetMsg msg;
    msg.type = NetMsg::MSGTYPE_DATA;
    msg.buffer = new char[head.bytes];
    if (nullptr != msg.buffer) {
      memcpy(msg.buffer, s_rbuffer, head.bytes);
      msg.size = head.bytes;

      msg_queue_->Push(msg);
    }
    ++rsequence;
  }

  client_->Close();
}
