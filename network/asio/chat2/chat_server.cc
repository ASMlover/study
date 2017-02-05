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
#include <cstdlib>
#include <iostream>
#include <memory>
#include <deque>
#include <vector>
#include <set>
#include <boost/asio.hpp>
#include "chat_protocol.h"

using boost::asio::ip::tcp;
using ChatMessageQueue = std::deque<ChatMessage>;

class ChatParticipant {
public:
  virtual ~ChatParticipant(void) {}
  virtual void deliver(const ChatMessage& msg) = 0;
};

using ChatParticipantPtr = std::shared_ptr<ChatParticipant>;

class ChatRoom : private boost::noncopyable {
  enum { RECENT_NMSGS_MAX = 100 };

  std::set<ChatParticipantPtr> participants_;
  ChatMessageQueue recent_msgs_;

  static std::atomic<std::int64_t> s_id_;
public:
  void join_in(const ChatParticipantPtr& participant) {
    participants_.insert(participant);
    for (const auto& msg : recent_msgs_)
      participant->deliver(msg);
  }

  void leave_out(const ChatParticipantPtr& participant) {
    participants_.erase(participant);
  }

  void deliver(const ChatMessage& msg) {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > RECENT_NMSGS_MAX)
      recent_msgs_.pop_front();

    for (const auto& participant : participants_)
      participant->deliver(msg);
  }

  static std::int64_t gen_id(void) {
    return ++s_id_;
  }
};

std::atomic<std::int64_t> ChatRoom::s_id_;

class ChatSession : public ChatParticipant, public std::enable_shared_from_this<ChatSession> {
  tcp::socket socket_;
  ChatRoom& chat_room_;
  ChatMessage readmsg_;
  ChatMessageQueue writmsg_queue_;

  void do_write_session(void) {
    char buf[64]{};
    std::snprintf(buf, sizeof(buf), "%08lld", ChatRoom::gen_id());
    ChatMessage msg(gen_chat_message(ChatProtocol::CP_SESSION, buf, std::strlen(buf)));
    deliver(msg);
  }

  void do_read_header(void) {
    auto self(shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(readmsg_.data(), ChatProtocol::NHEADER),
        [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (!ec && readmsg_.decode_header())
            do_read_body();
          else
            chat_room_.leave_out(shared_from_this());
        });
  }

  void do_read_body(void) {
    auto self(shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(readmsg_.body(), readmsg_.get_nbody()),
        [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (!ec) {
            chat_room_.deliver(readmsg_);
            do_read_header();
          }
          else {
            chat_room_.leave_out(shared_from_this());
          }
        });
  }

  void do_write(void) {
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
        boost::asio::buffer(writmsg_queue_.front().data(), writmsg_queue_.front().size()),
        [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (!ec) {
            writmsg_queue_.pop_front();
            if (!writmsg_queue_.empty())
              do_write();
          }
          else {
            chat_room_.leave_out(shared_from_this());
          }
        });
  }
public:
  ChatSession(tcp::socket&& socket, ChatRoom& chat_room)
    : socket_(std::move(socket))
    , chat_room_(chat_room) {
  }

  void start(void) {
    chat_room_.join_in(shared_from_this());
    do_write_session();
    do_read_header();
  }

  void deliver(const ChatMessage& msg) {
    bool write_in_progress = !writmsg_queue_.empty();
    writmsg_queue_.push_back(msg);
    if (!write_in_progress)
      do_write();
  }
};

class ChatServer : private boost::noncopyable {
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  ChatRoom chat_room_;

  void do_accept(void) {
    acceptor_.async_accept(socket_,
        [this](const boost::system::error_code& ec) {
          if (!ec)
            std::make_shared<ChatSession>(std::move(socket_), chat_room_)->start();

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

  try {
    boost::asio::io_service io_service;

    std::vector<std::unique_ptr<ChatServer>> servers;
    for (int i = 0; i < 5; ++i) {
      tcp::endpoint endpoint(tcp::v4(), 5555 + i);
      servers.emplace_back(new ChatServer(io_service, endpoint));
      servers[i]->start();
    }

    io_service.run();
  }
  catch (std::exception& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }

  return 0;
}
