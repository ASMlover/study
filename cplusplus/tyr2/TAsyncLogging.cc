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
#include <assert.h>
#include <stdio.h>
#include "TTimestamp.h"
#include "TLogFile.h"
#include "TAsyncLogging.h"

namespace tyr {

void AsyncLogging::thread_routine(void) {
  assert(running_);
  latch_.countdown();

  LogFile output(basename_, roll_size_, false);
  BufferPtr new_buffer1(new Buffer());
  BufferPtr new_buffer2(new Buffer());
  new_buffer1->bzero();
  new_buffer2->bzero();
  BufferVector buffers_to_write;
  buffers_to_write.reserve(16);
  while (running_) {
    assert(new_buffer1 && new_buffer1->length() == 0);
    assert(new_buffer2 && new_buffer2->length() == 0);
    assert(buffers_to_write.empty());

    {
      MutexGuard guard(mtx_);
      if (buffers_.empty())
        cond_.timed_wait(kFlushInterval_);
      buffers_.push_back(std::move(current_buffer_));
      buffers_to_write.swap(buffers_);
      if (!next_buffer_)
        next_buffer_ = std::move(new_buffer2);
    }

    assert(!buffers_to_write.empty())
    if (buffers_to_write.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof(buf), "dropped log messages at %s, %zd larger buffers\n",
          Timestamp::now().to_formatted_string().c_str(), buffers_to_write.size() - 2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      buffers_to_write.erase(buffers_to_write.begin() + 2, buffers_to_write.end());
    }

    for (size_t i = 0; i < buffers_to_write.size(); ++i)
      output.append(buffers_to_write[i]->data(), buffers_to_write[i]->length());

    if (buffers_to_write.size() > 2)
      buffers_to_write.resize(2);

    if (!new_buffer1) {
      assert(!buffers_to_write.empty());
      new_buffer1 = std::move(buffers_to_write.back());
      buffers_to_write.back();
      new_buffer1->reset();
    }

    if (!new_buffer2) {
      assert(!buffers_to_write.empty());
      new_buffer2 = std::move(buffers_to_write.back());
      buffers_to_write.back();
      new_buffer2->reset();
    }

    buffers_to_write.clear();
    output.flush();
  }

  output.flush();
}

AsyncLogging::AsyncLogging(const std::string& basename, size_t roll_size, int flush_interval)
  : kFlushInterval_(flush_interval)
  , running_(false)
  , basename_(basename)
  , roll_size_(roll_size)
  , thread_(std::bind(&AsyncLogging::thread_routine, this), "AsyncLogging")
  , latch_(1)
  , mtx_()
  , cond_(mtx_)
  , current_buffer_(new Buffer())
  , next_buffer_(new Buffer())
  , buffers_() {
  current_buffer_->bzero();
  next_buffer_->bzero();
  buffers_.reserve(16);
}

AsyncLogging::~AsyncLogging(void) {
  if (running_)
    stop();
}

void AsyncLogging::append(const char* buffer, int len) {
  MutexGuard guard(mtx_);
  if (current_buffer_->avail() > len) {
     current_buffer_->append(buffer, len);
  }
  else {
    buffers_.push_back(std::move(current_buffer_));
    if (next_buffer_)
      current_buffer_ = std::move(next_buffer_);
    else
      current_buffer_.reset(new Buffer());
    current_buffer_->append(buffer, len);
    cond_.notify();
  }
}

void AsyncLogging::start(void) {
  running_ = true;
  thread_.start();
  latch_.wait();
}

void AsyncLogging::stop(void) {
  running_ = false;
  cond_.notify();
  thread_.join();
}

}
