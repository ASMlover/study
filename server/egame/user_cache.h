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
#ifndef __USER_CACHE_HEADER_H__
#define __USER_CACHE_HEADER_H__

#include "user_data.h"

class UserCache : private util::UnCopyable {
  util::SmartPtr<redisContext> redis_;
public:
  UserCache(void);
  ~UserCache(void);

  bool Init(const char* addr, int port);
  void Destroy(void);
public:
  bool Get(const std::string& account, UserData& data);
  bool Set(const std::string& account, const UserData& data);
  bool Del(const std::string& account);

  bool SetUserID(const std::string& account, uint32_t user_id);
  bool SetUserName(
      const std::string& account, const std::string& user_name);
  bool SetGender(const std::string& account, UserData::GenderType gender);
  bool SetFace(const std::string& account, uint16_t face);
  bool SetLevel(const std::string& account, uint16_t level);
  bool SetExp(const std::string& account, uint64_t exp);
  bool SetScores(const std::string& account, uint32_t scores);
  bool SetCoins(const std::string& account, uint32_t coins);
  bool SetWinCount(const std::string& account, uint32_t win_count);
  bool SetLostCount(const std::string& account, uint32_t lost_count);
  bool SetFleeCount(const std::string& account, uint32_t flee_count);
  bool SetWinStreak(const std::string& account, uint32_t win_streak);
  bool SetPlayTime(const std::string& account, time_t play_time);
  bool SetLoginCount(const std::string& account, uint32_t lost_count);
  bool SetRegTime(const std::string& account, time_t reg_time);
  bool SetRegAddr(const std::string& account, uint32_t reg_addr);
  bool SetLastLoginTime(
      const std::string& account, time_t last_login_time);
  bool SetLastLoginAddr(
      const std::string& account, uint32_t last_login_addr);
private:
  bool Set(const char* format, ...);
};

#endif  //! __USER_CACHE_HEADER_H__
