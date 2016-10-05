// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TYR_ASYNCLOGGING_HEADER_H__
#define __TYR_ASYNCLOGGING_HEADER_H__

#include <memory>
#include <string>
#include <vector>
#include "TUnCopyable.h"
#include "TMutex.h"
#include "TCountdownLatch.h"
#include "TThread.h"
#include "TLogStream.h"

namespace tyr {

class AsyncLogging : private UnCopyable {
  typedef FixedBuffer<kLargeBuffer> Buffer;
  typedef std::unique_ptr<Buffer> BufferPtr;
  typedef std::vector<BufferPtr> BufferVector;

  const int kFlushInterval_;
  bool running_;
  std::string basename_;
  size_t roll_size_;
  Thread thread_;
  CountdownLatch latch_;
  mutable Mutex mtx_;
  Condition cond_;
  BufferPtr current_buffer_;
  BufferPtr next_buffer_;
  BufferVector buffers_;

  void thread_routine(void);
public:
  AsyncLogging(const std::string& basename, size_t roll_size, int flush_interval = 3);
  ~AsyncLogging(void);

  void append(const char* buffer, int len);
  void start(void);
  void stop(void);
};

}

#endif // __TYR_ASYNCLOGGING_HEADER_H__
