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
#include <algorithm>
#include <functional>
#include <memory>
#include <deque>
#include <set>
#include <vector>
#include <boost/asio.hpp>
#include "chat_message.h"

using boost::asio::ip::tcp;

typedef std::deque<ChatMessage> ChatMessageQueue;

class ChatParticipant {
public:
  virtual ~ChatParticipant(void) {}
  virtual void deliver(const ChatMessage& msg) = 0;
};

typedef std::shared_ptr<ChatParticipant> ChatParticipantPtr;

class ChatRoom : private boost::noncopyable {
  enum { MAX_RECENT_NMSG = 100 };
  std::set<ChatParticipantPtr> participants_;
  ChatMessageQueue recent_msgs_;
public:
  void join(const ChatParticipantPtr& participant) {
    participants_.insert(participant);
    std::for_each(recent_msgs_.begin(), recent_msgs_.end(),
        [this](const ChatMessage& msg) {
          deliver(msg);
        });
  }

  void leave(const ChatParticipantPtr& participant) {
    participants_.erase(participant);
  }

  void deliver(const ChatMessage& msg) {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > MAX_RECENT_NMSG)
      recent_msgs_.pop_front();

    std::for_each(participants_.begin(), participants_.end(),
        std::bind(&ChatParticipant::deliver, std::placeholders::_1, std::ref(msg)));
  }
};

class ChatSession : public ChatParticipant, public std::enable_shared_from_this<ChatSession> {
  tcp::socket socket_;
  ChatRoom& room_;
  ChatMessage rmsg_;
  ChatMessageQueue wmsg_queue_;

  void do_start(void) {
    auto self(shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(rmsg_.data(), rmsg_.get_nheader()),
        [this, self](const boost::system::error_code& ec, std::size_t /*read_bytes*/) {
          if (!ec && rmsg_.decode_header()) {
            boost::asio::async_read(socket_, boost::asio::buffer(rmsg_.body(), rmsg_.get_nbody()),
              [this, self](const boost::system::error_code& ec, std::size_t /*read_bytes*/) {
                if (!ec)
                  do_start();
                else
                  room_.leave(shared_from_this());
              });
          }
          else {
            room_.leave(shared_from_this());
          }
        });
  }

  void do_write(const boost::system::error_code& ec) {
    if (!ec) {
      wmsg_queue_.pop_front();
      if (!wmsg_queue_.empty()) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_,
            boost::asio::buffer(wmsg_queue_.front().data(), wmsg_queue_.front().size()),
            [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
              do_write(ec);
            });
      }
    }
    else {
      room_.leave(shared_from_this());
    }
  }
public:
  ChatSession(tcp::socket&& socket, ChatRoom& room)
    : socket_(std::move(socket))
    , room_(room) {
  }

  tcp::socket& get_socket(void) {
    return socket_;
  }

  void start(void) {
    room_.join(shared_from_this());
    do_start();
  }

  void deliver(const ChatMessage& msg) {
    bool write_in_progress = !wmsg_queue_.empty();
    wmsg_queue_.push_back(msg);
    if (!write_in_progress) {
      auto self(shared_from_this());
      boost::asio::async_write(socket_, boost::asio::buffer(wmsg_queue_.front().data(), wmsg_queue_.front().size()),
          [this, self](const boost::system::error_code& ec, std::size_t /*written_bytes*/) {
            do_write(ec);
          });
    }
  }
};

class ChatServer : private boost::noncopyable {
  tcp::acceptor acceptor_;
  tcp::socket socket_;

  void do_accept(void) {
    acceptor_.async_accept(socket_,
        [this](const boost::system::error_code& ec) {
          if (!ec)
            std::make_shared<ChatSession>(std::move(socket_))->start();

          do_accept();
        });
  }
public:
  ChatServer(boost::asio::io_service& io_service, const tcp::endpoint& endpoint)
    : acceptor_(io_service, endpoint)
    , socket_(io_service) {
  }

  void start(void) {
    do_accept();
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  boost::asio::io_service io_service;

  ChatServer server(io_service, tcp::endpoint(tcp::v4(), 5555));
  server.start();

  io_service.run();

  return 0;
}
