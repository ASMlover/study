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
#include <cerrno>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/Callbacks.h>
#include <Neptune/Channel.h>
#include <Neptune/EventLoop.h>
#include <Neptune/Connector.h>

namespace Neptune {

const int Connector::kMaxRetryDelayMillisecond;
const int Connector::kInitRetryDelayMillisecond;

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
  : loop_(loop)
  , retry_delay_millisecond_(kInitRetryDelayMillisecond)
  , server_addr_(server_addr) {
  CHAOSLOG_DEBUG << "Connector::Connector - [" << this << "]";
}

Connector::~Connector(void) {
  CHAOSLOG_DEBUG << "Connector::~Connector - [" << this << "]";
  CHAOS_CHECK(!channel_, "`channel_` should be invalid");
}

void Connector::start(void) {
  // TODO:
}

void Connector::stop(void) {
  // TODO:
}

void Connector::restart(void) {
  // TODO:
}

void Connector::start_in_loop(void) {
  // TODO:
}

void Connector::stop_in_loop(void) {
  // TODO:
}

void Connector::do_connect(void) {
  // TODO:
}

void Connector::do_connecting(int sockfd) {
  // TODO:
}

void Connector::do_handle_write(void) {
  // TODO:
}

void Connector::do_handle_error(void) {
  // TODO:
}

void Connector::retry(int sockfd) {
  // TODO:
}

void Connector::reset_channel(void) {
  // TODO:
}

int Connector::remove_and_reset_channel(void) {
  // TODO:
  return 0;
}

}
