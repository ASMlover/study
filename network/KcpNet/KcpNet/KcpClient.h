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

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include "KcpTypes.h"

namespace KcpNet {

using namespace boost::asio::ip;

class KcpClient : private boost::noncopyable {
  bool stopped_{};
  udp::socket socket_;
  boost::asio::deadline_timer timer_;
  bool connecting_{};
  std::uint64_t connect_begtime_{};
  bool connected_{};

  char readbuff_[1024 * 32]{};
  ikcpcb* kcp_{};
  CMessageFunctor message_fn_{};

  static int output_handler(const char* buf, int len, ikcpcb* kcp, void* user);
  void init_kcp(kcp_conv_t conv);
  void update(void);
  void do_read_connection(void);
  void do_write_connection(void);
  void do_timer(void);
  void do_read(void);
  void write_udp_buffer(const char* buf, int len);
public:
  KcpClient(boost::asio::io_service& io_service, std::uint16_t bind_port);
  void stop(void);
  void connect_async(const std::string& remote_ip, std::uint16_t remote_port);
  void write_buffer(const std::string& buf);

  bool is_connecting(void) const {
    return connecting_;
  }

  bool is_connected(void) const {
    return connected_;
  }

  void bind_meesage_functor(const CMessageFunctor& fn) {
    message_fn_ = fn;
  }

  void bind_meesage_functor(CMessageFunctor&& fn) {
    message_fn_ = std::move(fn);
  }
};

}
