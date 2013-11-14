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
#ifndef __SELECT_NETWORK_HADER_H__
#define __SELECT_NETWORK_HADER_H__

#include <vector>

class SelectPoll;
class SelectWorker;
class SelectListener;
struct EventHandler;
class SelectNetwork {
  enum {
    kDefaultWorkerCount = 1, 
    kDefaultBufferSize  = 16 * 1024, 
  };
  EventHandler* handler_;
  SelectPoll*   poll_;
  SelectWorker* workers_;
  int           worker_count_;
  std::vector<SelectListener*> listeners_;

  SelectNetwork(const SelectNetwork&);
  SelectNetwork& operator =(const SelectNetwork&);
public:
  explicit SelectNetwork(void);
  ~SelectNetwork(void);

  inline void Attach(EventHandler* handler)
  {
    handler_ = handler;
  }
public:
  bool Init(int worker_count = kDefaultWorkerCount, 
      int rbytes = kDefaultBufferSize, 
      int wbytes = kDefaultBufferSize);
  void Destroy(void);

  bool Listen(const char* ip = "127.0.0.1", unsigned short port = 5555);
};

#endif  //! __SELECT_NETWORK_HADER_H__
