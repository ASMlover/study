//! Copyright (c) 2014 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include "global.h"
#include "user_data.h"
#include "user_cache.h"




UserCache::UserCache(void)
  : redis_(static_cast<redisContext*>(NULL)) {
}

UserCache::~UserCache(void) {
}

bool UserCache::Init(const char* addr, int port) {
  struct timeval tv = {1, 500000};
  redis_ = util::SmartPtr<redisContext>(
      redisConnectWithTimeout(addr, port, tv), 
      redisFree);

  if (NULL == redis_.Get() || 0 != redis_->err) {
    fprintf(stderr, "Connect to redis-server %s:%d FAILED\n", addr, port);
    return false;
  }

  return true;
}

void UserCache::Destroy(void) {
}


bool UserCache::Get(const std::string& account, UserData& data) {
  return true;
}

bool UserCache::Set(const std::string& account, const UserData& data) {
  return true;
}

bool UserCache::Del(const std::string& account) {
  return true;
}
