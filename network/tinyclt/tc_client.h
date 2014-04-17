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
#ifndef __TINYCLT_CLIENT_HEADER_H__
#define __TINYCLT_CLIENT_HEADER_H__

#define CLIENT_DISP_CALLBACK(__selector__, __target__)\
  std::bind(&__selector__, &(__target__), \
      std::placeholders::_1, std::placeholders::_2)

class Socket;
class Thread;
class Client : private UnCopyable {
  typedef std::function<bool (const void*, uint32_t)> DispatcherType;

  bool connected_;
  uint32_t wsequence_;
  DispatcherType dispatcher_;
  std::shared_ptr<Socket> client_;
  std::shared_ptr<Thread> thread_;
public:
  Client(void);
  ~Client(void);

  inline bool IsConnected(void) const {
    return connected_;
  }

  bool Connect(const char* address, 
      uint16_t port, DispatcherType dispatcher);
  void Disconnect(void);

  bool Write(const void* buffer, uint32_t bytes);
private:
  void ReadRoutine(void* argument);
};

#endif  // __TINYCLT_CLIENT_HEADER_H__
