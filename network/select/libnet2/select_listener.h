//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#ifndef __SELECT_LISTENER_HEADER_H__
#define __SELECT_LISTENER_HEADER_H__


class Thread;
class Socket;
class SelectPoll;
struct EventHandler;
class SelectListener {
  bool    running_;
  Thread* thread_;
  Socket* listener_;
  SelectPoll* poll_;
  EventHandler* handler_;

  SelectListener(const SelectListener&);
  SelectListener& operator =(const SelectListener&);
public:
  explicit SelectListener(void);
  ~SelectListener(void);

  inline void Attach(SelectPoll* poll)
  {
    poll_ = poll;
  }

  inline void Attach(EventHandler* handler)
  {
    handler_ = handler;
  }
public:
  bool Start(const char* ip, unsigned short port);
  void Stop(void);
private:
  static void Routine(void* argument);
};

#endif  //! __SELECT_LISTENER_HEADER_H__
