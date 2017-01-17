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
#ifndef NEPTUNE_CHANNEL_H
#define NEPTUNE_CHANNEL_H

#include <functional>
#include <memory>
#include <string>
#include <Chaos/UnCopyable.h>
#include <Chaos/Datetime/Timestamp.h>

namespace Neptune {

class EventLoop;

class Channel : private Chaos::UnCopyable {
  using EventFunction = std::function<void (void)>;
  using ReadEventFunction = std::function<void (Chaos::Timestamp)>;

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

  ReadEventFunction read_fn_{};
  EventFunction write_fn_{};
  EventFunction close_fn_{};
  EventFunction error_fn_{};

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  void update(void);
  void handle_event_with_guard(Chaos::Timestamp recvtime);
  static std::string events_to_string(int fd, int event);
public:
  Channel(EventLoop* loop, int fd);
  ~Channel(void);

  void handle_event(Chaos::Timestamp recvtime);
  void tie(const std::shared_ptr<void>& tiep);
  void remove(void);

  std::string events_to_string(void) const;
  std::string revents_to_string(void) const;

  void bind_read_functor(const ReadEventFunction& fn) {
    read_fn_ = fn;
  }

  void bind_read_functor(ReadEventFunction&& fn) {
    read_fn_ = std::move(fn);
  }

  void bind_write_functor(const EventFunction& fn) {
    write_fn_ = fn;
  }

  void bind_write_functor(EventFunction&& fn) {
    write_fn_ = std::move(fn);
  }

  void bind_close_functor(const EventFunction& fn) {
    close_fn_ = fn;
  }

  void bind_close_functor(EventFunction&& fn) {
    close_fn_ = std::move(fn);
  }

  void bind_error_functor(const EventFunction& fn) {
    error_fn_ = fn;
  }

  void bind_error_functor(EventFunction&& fn) {
    error_fn_ = std::move(fn);
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

  void set_index(int index) {
    index_ = index;
  }

  int get_index(void) const {
    return index_;
  }

  void set_non_loghup(void) {
    log_hup_ = false;
  }

  EventLoop* get_loop(void) const {
    return loop_;
  }
};

}

#endif // NEPTUNE_CHANNEL_H
