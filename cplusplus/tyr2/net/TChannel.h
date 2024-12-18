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
#ifndef __TYR_NET_CHANNEL_HEADER_H__
#define __TYR_NET_CHANNEL_HEADER_H__

#include <functional>
#include <memory>
#include <string>
#include "../basic/TUnCopyable.h"
#include "../basic/TTimestamp.h"

namespace tyr { namespace net {

class EventLoop;

typedef std::function<void (void)> EventCallback;
typedef std::function<void (basic::Timestamp)> ReadEventCallback;

class Channel : private basic::UnCopyable {
  EventLoop* loop_{};
  const int fd_{};
  int events_{};
  int revents_{};
  int index_{-1};
  bool log_hup_{true};
  bool event_handling_{};
  bool added_to_loop_{};
  bool tied_{};
  std::weak_ptr<void> wk_tie_;

  ReadEventCallback read_fn_{};
  EventCallback write_fn_{};
  EventCallback error_fn_{};
  EventCallback close_fn_{};

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  void update(void);
  void handle_event_with_guard(basic::Timestamp recv_time);

  static std::string events_to_string(int fd, int event);
public:
  Channel(EventLoop* loop, int fd);
  ~Channel(void);

  void handle_event(basic::Timestamp recv_time);
  void tie(const std::shared_ptr<void>& tiep);
  void remove(void);

  // for debuging
  std::string events_to_string(void) const;
  std::string revents_to_string(void) const;

  void set_read_callback(const ReadEventCallback& fn) {
    read_fn_ = fn;
  }

  void set_read_callback(ReadEventCallback&& fn) {
    read_fn_ = std::move(fn);
  }

  void set_write_callback(const EventCallback& fn) {
    write_fn_ = fn;
  }

  void set_write_callback(EventCallback&& fn) {
    write_fn_ = std::move(fn);
  }

  void set_error_callback(const EventCallback& fn) {
    error_fn_ = fn;
  }

  void set_error_callback(EventCallback&& fn) {
    error_fn_ = std::move(fn);
  }

  void set_close_callback(const EventCallback& fn) {
    close_fn_ = fn;
  }

  void set_close_callback(EventCallback&& fn) {
    close_fn_ = std::move(fn);
  }

  int get_fd(void) const {
    return fd_;
  }

  int get_events(void) const {
    return events_;
  }

  void set_revents(int revents) {
    revents_ = revents;
  }

  bool is_none_event(void) const {
    return events_ == kNoneEvent;
  }

  void enabled_reading(void) {
    events_ |= kReadEvent;
    update();
  }

  void disabled_reading(void) {
    events_ &= ~kReadEvent;
    update();
  }

  void enabled_writing(void) {
    events_ |= kWriteEvent;
    update();
  }

  void disabled_writing(void) {
    events_ &= ~kWriteEvent;
    update();
  }

  void disabled_all(void) {
    events_ = kNoneEvent;
    update();
  }

  bool is_reading(void) const {
    return events_ & kReadEvent;
  }

  bool is_writing(void) const {
    return events_ & kWriteEvent;
  }

  int get_index(void) const {
    return index_;
  }

  void set_index(int index) {
    index_ = index;
  }

  void set_non_loghup(void) {
    log_hup_ = false;
  }

  EventLoop* get_owner_loop(void) const {
    return loop_;
  }
};

}}

#endif // __TYR_NET_CHANNEL_HEADER_H__
