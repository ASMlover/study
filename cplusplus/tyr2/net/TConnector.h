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
#ifndef __TYR_NET_CONNECTOR_HEADER_H__
#define __TYR_NET_CONNECTOR_HEADER_H__

#include <functional>
#include <memory>
#include "../basic/TUnCopyable.h"
#include "TInetAddress.h"
#include "TTimer.h"

namespace tyr { namespace net {

class Channel;
class EventLoop;

class Connector : private basic::UnCopyable {
  typedef std::function<void (int sockfd)> NewConnectionCallback;

  enum States {
    STATES_DISCONNECTED,
    STATES_CONNECTING,
    STATES_CONNECTED,
  };
  static const int kMaxRetryDelayMillisecond = 30 * 1000;
  static const int kInitRetryDelayMillisecond = 500;

  EventLoop* loop_{};
  bool connect_{};
  States state_{};
  int retry_delay_ms_{};
  InetAddress server_addr_;
  std::unique_ptr<Channel> channel_;
  NewConnectionCallback new_connection_fn_;
  TimerID timerid_;

  void set_state(States s) {
    state_ = s;
  }

  void start_in_loop(void);
  void connect(void);
  void connecting(int sockfd);
  void handle_write(void);
  void handle_error(void);
  void retry(int sockfd);
  int remove_and_reset_channel(void);
  void reset_channel(void);
public:
  Connector(EventLoop* loop, const InetAddress& server_addr);
  ~Connector(void);

  void set_new_connection_callback(const NewConnectionCallback& fn) {
    new_connection_fn_ = fn;
  }

  const InetAddress& get_server_address(void) const {
    return server_addr_;
  }

  void start(void);
  void restart(void);
  void stop(void);
};
typedef std::shared_ptr<Connector> ConnectorPtr;

}}

#endif // __TYR_NET_CONNECTOR_HEADER_H__
