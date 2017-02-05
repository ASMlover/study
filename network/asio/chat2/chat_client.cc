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

  void do_read_header(void) {
    boost::asio::async_read(socket_, boost::asio::buffer(readmsg_.data(), ChatProtocol::NHEADER),
        [this](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (!ec && readmsg_.decode_header())
            do_read_body();
          else
            socket_.close();
        });
  }

  void do_read_body(void) {
    boost::asio::async_read(socket_, boost::asio::buffer(readmsg_.body(), readmsg_.get_nbody()),
        [this](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (!ec) {
            if (readmsg_.get_proto() == ChatProtocol::CP_SESSION) {
              session_id_ = readmsg_.body();
            }
            else {
              std::cout.write(readmsg_.body(), readmsg_.get_nbody()); std::cout << std::endl;
              do_read_header();
            }
          }
          else {
            socket_.close();
          }
        });
  }

  void write_message(const ChatMessage& msg) {
    io_service_.post([this, msg](void) {
          bool write_in_progress = !writmsg_queue_.empty();
          writmsg_queue_.push_back(msg);
          if (!write_in_progress)
            do_write();
        });
  }

  void do_write(void) {
    boost::asio::async_write(socket_,
        boost::asio::buffer(writmsg_queue_.front().data(), writmsg_queue_.front().size()),
        [this](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (!ec) {
            writmsg_queue_.pop_front();
            if (!writmsg_queue_.empty())
              do_write();
          }
          else {
            socket_.close();
          }
        });
  }
public:
  ChatClient(boost::asio::io_service& io_service)
    : io_service_(io_service)
    , socket_(io_service_) {
  }

  void start(tcp::resolver::iterator endpoint_iter) {
    boost::asio::async_connect(socket_, endpoint_iter,
        [this](const boost::system::error_code& ec, tcp::resolver::iterator) {
          if (!ec)
            do_read_header();
          else
            socket_.close();
        });
  }

  void write(const char* buf) {
    char writbuf[1024]{};
    std::snprintf(writbuf, sizeof(writbuf), "[%s] - %s", session_id_.c_str(), buf);
    ChatMessage msg(gen_chat_message(ChatProtocol::CP_MESSAGE, writbuf, std::strlen(writbuf)));
    write_message(msg);
  }

  void close(void) {
    io_service_.post([this](void) { socket_.close(); });
  }

  const std::string& get_session(void) const {
    return session_id_;
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  return 0;
}
