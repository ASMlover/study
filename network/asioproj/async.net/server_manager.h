// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <mutex>
#include <thread>
#include <set>
#include <vector>
#include <boost/asio.hpp>
#include "server_base.h"

using WorkPtr = std::shared_ptr<boost::asio::io_service::work>;
using ThreadPtr = std::shared_ptr<std::thread>;
using ThreadPool = std::vector<ThreadPtr>;

class ServerManager : private boost::noncopyable {
  boost::asio::io_service io_service_;
  std::size_t nthreads_{};
  WorkPtr work_;
  mutable std::mutex mutex_;
  std::set<ServerPtr> servers_;

  ServerManager(void);
  ~ServerManager(void);
public:
  void add_server(const ServerPtr& server);
  void start(void);
  void stop(void);
  void set_worker(void);
  void unset_worker(void);

  void set_nthreads(std::size_t nthreads) {
    nthreads_ = nthreads;
  }

  std::size_t get_nthreads(void) const {
    return nthreads_;
  }

  boost::asio::io_service& get_io_service(void) {
    return io_service_;
  }

  static ServerManager& get_instance(void) {
    static ServerManager ins;
    return ins;
  }
};
