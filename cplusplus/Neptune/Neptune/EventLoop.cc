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
#include <Chaos/Logging/Logging.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/IgnoreSigPipe.h>
// #include <Neptune/Channel.h>
#include <Neptune/Poller.h>
// #include <Neptune/TimerQueue.h>
// #include <Neptune/WakeupSignaler.h>
#include <Neptune/EventLoop.h>

namespace Neptune {

__chaos_tl EventLoop* t_loopthread = nullptr;
const int kPollMicrosecond = 10000;

Neptune::IgnoreSigPipe g_ignore_sigpipe;

// bool looping_{}; // need atomic
// bool quit_{}; // need atomic
// bool event_handing_{}; // need atomic
// bool calling_pending_functors_{}; // need atomic
// std::int64_t iteration_{};
// const pid_t tid_{};
// Chaos::Timestamp poll_return_time_;
// std::unique_ptr<Poller> poller_;
// std::unique_ptr<TimerQueue> timer_queue_;
// // NetInitializer<> init_;
// std::unique_ptr<WakeupSignaler> wakeup_;
// std::unique_ptr<Channel> wakeup_channel_;
// Chaos::Any context_;
// Channel* current_active_channel_{};
// std::vector<Channel*> active_channels_;
// mutable Chaos::Mutex mutex_;
// std::vector<PendFunction> pending_functors_; // locked by mutex_
EventLoop::EventLoop(void)
  : tid_(Chaos::CurrentThread::get_tid())
  // , poller_(Poller::get_poller(this))
  // , timer_queue_(new TimerQueue(this))
  // , wakeup_(new WakeupSignaler())
  // , wakeup_channel_(new Channel(this, wakeup_->get_reader()))
{
  CHAOSLOG_DEBUG << "EventLoop::EventLoop - created " << this << " in thread " << tid_;
  if (t_loopthread)
    CHAOSLOG_SYSFATAL << "EventLoop::EventLoop - another EventLoop " << t_loopthread << " exists in thread " << tid_;
  else
    t_loopthread = this;

  // TODO:
}

EventLoop::~EventLoop(void) {
  // TODO:
}

void EventLoop::loop(void) {
  // TODO:
}

void EventLoop::quit(void) {
  // TODO:
}

void EventLoop::wakeup(void) {
  // TODO:
}

void EventLoop::cancel(Neptune::TimerID timerid) {
  // TODO:
}

void EventLoop::update_channel(Channel* channel) {
  // TODO:
}

void EventLoop::remove_channel(Channel* channel) {
  // TODO:
}

bool EventLoop::has_channel(Channel* channel) {
  // TODO:
  return false;
}

std::size_t EventLoop::get_functor_count(void) const {
  // TODO:
  return 0;
}

Neptune::TimerID EventLoop::run_at(Chaos::Timestamp time, const Neptune::TimerCallback& fn) {
  // TODO:
  return TimerID();
}

Neptune::TimerID EventLoop::run_at(Chaos::Timestamp time, Neptune::TimerCallback&& fn) {
  // TODO:
  return TimerID();
}

Neptune::TimerID EventLoop::run_after(double delay, const Neptune::TimerCallback& fn) {
  // TODO:
  return TimerID();
}

Neptune::TimerID EventLoop::run_after(double delay, Neptune::TimerCallback&& fn) {
  // TODO:
  return TimerID();
}

Neptune::TimerID EventLoop::run_every(double interval, const Neptune::TimerCallback& fn) {
  // TODO:
  return TimerID();
}

Neptune::TimerID EventLoop::run_every(double interval, Neptune::TimerCallback&& fn) {
  // TODO:
  return TimerID();
}

void EventLoop::run_in_loop(const PendFunction& fn) {
  // TODO:
}

void EventLoop::run_in_loop(PendFunction&& fn) {
  // TODO:
}

void EventLoop::put_in_loop(const PendFunction& fn) {
  // TODO:
}

void EventLoop::put_in_loop(PendFunction&& fn) {
  // TODO:
}

EventLoop* EventLoop::get_loop_in_currentthread(void) {
  return t_loopthread;
}

void EventLoop::abort_not_in_loopthread(void) {
  // TODO:
}

void EventLoop::do_handle_read(void) {
  // TODO:
}

void EventLoop::do_pending_functors(void) {
  // TODO:
}

void EventLoop::debug_active_channels(void) const {
  // TODO:
}

}
