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
#include <atomic>
#include <algorithm>
#include <functional>
#include <memory>
#include <deque>
#include <set>
#include <sstream>
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

  static std::atomic<std::int64_t> s_id_;
public:
  void join(const ChatParticipantPtr& participant) {
    participants_.insert(participant);
    for (const auto& msg : recent_msgs_)
      participant->deliver(msg);
  }

  void leave(const ChatParticipantPtr& participant) {
    participants_.erase(participant);
  }

  void deliver(const ChatMessage& msg) {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > MAX_RECENT_NMSG)
      recent_msgs_.pop_front();

    for (auto& participant : participants_)
      participant->deliver(msg);
  }

  static std::int64_t gen_id(void) {
    return ++s_id_;
  }

  static std::string gen_session_id(void) {
    std::stringstream ss;
    return ss << gen_id(), ss.str();
  }
};

class ChatSession : public ChatParticipant, public std::enable_shared_from_this<ChatSession> {
  tcp::socket socket_;
  ChatRoom& room_;
  ChatMessage rmsg_;
  ChatMessageQueue wmsg_queue_;

  void do_read_header(void) {
    auto self(shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(rmsg_.data(), rmsg_.get_nheader()),
        [this, self](const boost::system::error_code& ec, std::size_t /*read_bytes*/) {
          if (!ec && rmsg_.decode_header())
            do_read_body();
          else
            room_.leave(shared_from_this());
        });
  }

  void do_read_body(void) {
    auto self(shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(rmsg_.body(), rmsg_.get_nbody()),
        [this, self](const boost::system::error_code& ec, std::size_t /*read_bytes*/) {
          if (!ec) {
            room_.deliver(rmsg_);
            do_read_header();
          }
          else {
            room_.leave(shared_from_this());
          }
        });
  }

  void do_write(void) {
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
        boost::asio::buffer(wmsg_queue_.front().data(), wmsg_queue_.front().size()),
        [this, self](const boost::system::error_code& ec, std::size_t /*written_bytes*/) {
          if (!ec) {
            wmsg_queue_.pop_front();
            if (!wmsg_queue_.empty())
              do_write();
          }
          else {
            room_.leave(shared_from_this());
          }
        });
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
    do_read_header();
  }

  void deliver(const ChatMessage& msg) {
    bool write_in_progress = !wmsg_queue_.empty();
    wmsg_queue_.push_back(msg);
    if (!write_in_progress)
      do_write();
  }
};

class ChatServer : private boost::noncopyable {
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  ChatRoom room_;

  void do_accept(void) {
    acceptor_.async_accept(socket_,
        [this](const boost::system::error_code& ec) {
          if (!ec)
            std::make_shared<ChatSession>(std::move(socket_), room_)->start();

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
