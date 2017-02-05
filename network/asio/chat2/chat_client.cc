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
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <deque>
#include <thread>
#include <boost/asio.hpp>
#include "chat_protocol.h"

using boost::asio::ip::tcp;
using ChatMessageQueue = std::deque<ChatMessage>;

class ChatClient : private boost::noncopyable {
  boost::asio::io_service& io_service_;
  tcp::socket socket_;
  ChatMessage readmsg_;
  ChatMessageQueue writmsg_queue_;
  std::string session_id_;

  void do_connect(tcp::resolver::iterator endpoint_iter) {
    // TODO:
  }

  void do_read_header(void) {
    // TODO:
  }

  void do_read_body(void) {
    // TODO:
  }

  void do_write(void) {
    // TODO:
  }
public:
  ChatClient(boost::asio::io_service& io_service)
    : io_service_(io_service)
    , socket_(io_service_) {
  }

  void start(tcp::resolver::iterator endpoint_iter) {
    // TODO:
  }

  void write(const char* buf, std::size_t len) {
    // TODO:
  }

  void write(const ChatMessage& msg) {
    // TODO:
  }

  void close(void) {
    io_service_.post([this](void) { socket_.close(); });
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  return 0;
}
