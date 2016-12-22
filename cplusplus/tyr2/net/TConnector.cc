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
#include <errno.h>
#include "../basic/TLogging.h"
#include "TChannel.h"
#include "TEventLoop.h"
#include "TSocketSupport.h"
#include "TConnector.h"

namespace tyr { namespace net {

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
  : loop_(loop)
  , server_addr_(server_addr)
  , retry_delay_ms_(kInitRetryDelayMicrosecond) {
  TYRLOG_DEBUG << "Connector ctor [" << this << "]";
}

Connector::~Connector(void) {
  TYRLOG_DEBUG << "Connector dtor [" << this << "]";
  loop_->cancel(timerid_);
  assert(!channel_);
}

void Connector::start(void) {
  connect_ = true;
  loop_->run_in_loop(std::bind(&Connector::start_in_loop, this));
}

// void restart(void);
// void stop(void);

void Connector::start_in_loop(void) {
}

// void connect(void);

void Connector::connecting(int sockfd) {
  set_state(STATES_CONNECTING);
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));
  channel_->set_write_callback(std::bind(&Connector::handle_write, this));
  channel_->set_error_callback(std::bind(&Connector::handle_error, this));
  channel_->enabled_writing();
}

// void retry(int sockfd);
// int remove_and_reset_channel(void);
// void reset_channel(void);

void Connector::handle_write(void) {
}

void Connector::handle_error(void) {
}

}}
