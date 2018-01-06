// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "ext_timer.h"
#include "ext_iomgr.h"
#include "ext_timermgr.h"

namespace ext {

Timer::Timer(std::uint32_t id,
    double delay, long sec, long millisec, bool is_repeat)
  : id_(id)
  , raw_delay_(delay)
  , is_repeat_(is_repeat)
  , dealy_(boost::posix_time::seconds(sec) + boost::posix_time::milliseconds(millisec))
  , timer_(IOMgr::instance().get_io_service())
{
}

Timer::~Timer(void) {
}

void Timer::start(void) {
  auto self(shared_from_this());
  timer_.expires_from_now(dealy_);
  timer_.async_wait([this, self](const boost::system::error_code& ec) {
        if (!ec) {
          {
            std::unique_lock<std::mutex> g(mutex_);
            if (is_cleared_)
              return;

            // TODO:

            if (is_repeat_)
              start();
            else
              is_cleared_ = true;
          }
          if (is_cleared_)
            TimerMgr::instance().unreg(id_);
        }
      });
}

void Timer::stop(void) {
  std::unique_lock<std::mutex> g(mutex_);
  if (!is_cleared_) {
    timer_.cancel();
    is_cleared_ = true;
  }
}

}
