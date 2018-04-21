// Copyright (c) 2018 ASMlover. All rights reserved.
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
#pragma once

#include <memory>
#include <thread>
#include <boost/asio.hpp>

namespace nyx { namespace core {

using thread_ptr = std::shared_ptr<std::thread>;
using work_ptr = std::shared_ptr<boost::asio::io_service::work>;

class io_manager : private boost::noncopyable {
  bool running_{};
  std::size_t thread_num_{};
  std::uint64_t loop_time_{};
  boost::asio::io_service io_service_;
  work_ptr work_;
  thread_ptr thread_;

  io_manager(void);
  ~io_manager(void);

  void thread_run(void);
public:
  static io_manager& instance(void) {
    static io_manager ins;
    return ins;
  }

  void set_thread_number(std::size_t num) {
    thread_num_ = num;
  }

  void set_loop_time(std::uint64_t loop_time) {
    loop_time_ = loop_time;
  }

  std::uint64_t get_loop_time(void) const {
    return loop_time_;
  }

  void start(void);
  void stop(void);
  bool poll(bool no_sleep);
  void set_work(void);
  void unset_work(void);
};

}}
