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
};

std::atomic<std::int64_t> ChatRoom::s_id_;
