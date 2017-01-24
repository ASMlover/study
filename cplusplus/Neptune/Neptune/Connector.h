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
#ifndef NEPTUNE_CONNECTOR_H
#define NEPTUNE_CONNECTOR_H

#include <functional>
#include <memory>
#include <Chaos/UnCopyable.h>
#include <Neptune/InetAddress.h>
#include <Neptune/Timer.h>

namespace Neptune {

class Channel;
class EventLoop;

class Connector : private Chaos::UnCopyable, public std::enable_shared_from_this<Connector> {
  using NewConnectionFunction = std::function<void (int sockfd)>;

  enum class NetLink {
    NETLINK_DISCONNECTED,
    NETLINK_CONNECTING,
    NETLINK_CONNECTED,
  };
  static const int kMaxRetryDelayMillisecond = 30 * 1000;
  static const int kInitRetryDelayMillisecond = 500;

  EventLoop* loop_{};
  bool need_connect_{};
  NetLink linkstate_{};
  int retry_delay_millisecond_{};
  InetAddress server_addr_;
  std::unique_ptr<Channel> channel_;
  NewConnectionFunction new_connection_fn_{};

  void start_in_loop(void);
  void do_connect(void);
  void do_connecting(int sockfd);
  void do_handle_write(void);
  void do_handle_error(void);
  void retry(int sockfd);
  int remove_and_reset_channel(void);
  const char* linkstate_to_string(void) const;

  void set_linkstate(NetLink linkstate) {
    linkstate_ = linkstate;
  }
public:
  Connector(EventLoop* loop, const InetAddress& server_addr);
  ~Connector(void);

  void start(void);
  void stop(void);
  void restart(void);

  void bind_new_connection_functor(const NewConnectionFunction& fn) {
    new_connection_fn_ = fn;
  }

  void bind_new_connection_functor(NewConnectionFunction&& fn) {
    new_connection_fn_ = std::move(fn);
  }

  const InetAddress& get_server_address(void) const {
    return server_addr_;
  }
};

}

#endif // NEPTUNE_CONNECTOR_H
