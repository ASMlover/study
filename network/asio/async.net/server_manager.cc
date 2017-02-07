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
#include "server_manager.h"

ServerManager::ServerManager(void)
  : nthreads_(8) {
}

ServerManager::~ServerManager(void) {
}

void ServerManager::add_server(const ServerPtr& server) {
  std::unique_lock<std::mutex> guard(mutex_);
  servers_.insert(server);
}

void ServerManager::start(void) {
  ThreadPool pool;
  for (std::size_t i = 0; i < nthreads_; ++i) {
    ThreadPtr t(new std::thread([this] { io_service_.run(); }));
    pool.emplace_back(t);
  }

  for (auto& t : pool)
    t->join();
}

void ServerManager::stop(void) {
  std::unique_lock<std::mutex> guard(mutex_);
  for (auto& s : servers_)
    s->stop();
  servers_.clear();
}

void ServerManager::set_worker(void) {
  std::unique_lock<std::mutex> guard(mutex_);
  work_.reset(new boost::asio::io_service::work(io_service_));
}

void ServerManager::unset_worker(void) {
  std::unique_lock<std::mutex> guard(mutex_);
  work_.reset();
}
