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
#include "connection_manager.h"

ConnectionManager::ConnectionManager(void) {
}

ConnectionManager::~ConnectionManager(void) {
}

void ConnectionManager::register_connection(const ConnectionPtr& conn) {
  std::unique_lock<std::mutex> guard(mutex_);
  connections_.insert(conn);
  ++nregistered_;
}

void ConnectionManager::unregister_connection(const ConnectionPtr& conn) {
  std::unique_lock<std::mutex> guard(mutex_);
  connections_.erase(conn);
}

void ConnectionManager::stop_all(void) {
  std::unique_lock<std::mutex> guard(mutex_);
  for (auto& conn : connections_)
    conn->do_stop();
}

bool ConnectionManager::has_connection(const ConnectionPtr& conn) const {
  std::unique_lock<std::mutex> guard(mutex_);
  return connections_.find(conn) != connections_.end();
}
