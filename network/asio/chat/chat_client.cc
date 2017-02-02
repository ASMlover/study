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
#include <iostream>
#include <deque>
#include <vector>
#include <thread>
#include <boost/asio.hpp>
#include "chat_message.h"

using boost::asio::ip::tcp;
using ChatMessageQueue = std::deque<ChatMessage>;

class ChatClient : private boost::noncopyable {
  boost::asio::io_service& io_service_;
  tcp::socket socket_;
  ChatMessage rmsg_;
  ChatMessageQueue wmsg_queue_;

  void do_connect(tcp::resolver::iterator endpoint_iter) {
    boost::asio::async_connect(socket_, endpoint_iter,
        [this](const boost::system::error_code& ec, tcp::resolver::iterator) {
          if (!ec)
            do_read_header();
        });
  }

  void do_read_header(void) {
    boost::asio::async_read(socket_, boost::asio::buffer(rmsg_.data(), rmsg_.get_nheader()),
        [this](const boost::system::error_code& ec, std::size_t /*read_bytes*/) {
          if (!ec && rmsg_.decode_header())
            do_read_body();
          else
            socket_.close();
        });
  }

  void do_read_body(void) {
    boost::asio::async_read(socket_, boost::asio::buffer(rmsg_.body(), rmsg_.get_nbody()),
        [this](const boost::system::error_code& ec, std::size_t /*read_bytes*/) {
          if (!ec) {
            std::cout.write(rmsg_.body(), rmsg_.get_nbody()); std::cout << std::endl;
            do_read_header();
          }
          else {
            socket_.close();
          }
        });
  }

  void do_write(void) {
    boost::asio::async_write(socket_,
        boost::asio::buffer(wmsg_queue_.front().data(), wmsg_queue_.front().size()),
        [this](const boost::system::error_code& ec, std::size_t /*written_bytes*/) {
          if (!ec) {
            wmsg_queue_.pop_front();
            if (!wmsg_queue_.empty())
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
    , socket_(io_service) {
  }

  void start(tcp::resolver::iterator endpoint_iter) {
    do_connect(endpoint_iter);
  }

  void write(const ChatMessage& msg) {
    io_service_.post(
        [this, msg](void) {
          bool write_in_progress = !wmsg_queue_.empty();
          wmsg_queue_.push_back(msg);
          if (!write_in_progress)
            do_write();
        });
  }

  void close(void) {
    io_service_.post([this](void) { socket_.close(); });
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  boost::asio::io_service io_service;

  tcp::resolver r(io_service);
  auto endpoint_iter = r.resolve({"127.0.0.1", "5555"});
  ChatClient client(io_service);
  client.start(endpoint_iter);

  std::thread t([&io_service](void) { io_service.run(); });
  char line[MAX_NBODY + 1]{};
  while (std::cin.getline(line, MAX_NBODY + 1)) {
    ChatMessage msg;
    msg.set_nbody(std::strlen(line));
    std::memcpy(msg.body(), line, msg.get_nbody());
    msg.encode_header();
    client.write(msg);
  }

  client.close();
  t.join();

  return 0;
}
