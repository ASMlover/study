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
  util::SmartPtr<redisReply> reply(
      static_cast<redisReply*>(redisCommand(
          redis_.Get(), "exists %s", account.c_str())), 
      freeReplyObject);
  if (NULL == reply.Get())
    return false;

  if (REDIS_REPLY_INTEGER != reply->type)
    return false;
  if (0 == reply->integer)
    return false;

  reply = util::SmartPtr<redisReply>(
      static_cast<redisReply*>(redisCommand(
          redis_.Get(), 
          "hmget %s " 
          "account "
          "user_id "
          "user_name "
          "gender "
          "face "
          "level "
          "exp "
          "scores "
          "coins "
          "win_count "
          "lost_count "
          "flee_count " 
          "win_streak "
          "play_time "
          "login_count "
          "reg_time "
          "reg_addr " 
          "last_login_time "
          "last_login_addr", 
          account.c_str())), 
      freeReplyObject);
  if (REDIS_REPLY_ARRAY != reply->type)
    return false;

  data.account          = reply->element[0]->str;
  data.user_id          = atoi(reply->element[1]->str);
  data.user_name        = reply->element[2]->str;
  data.gender           = static_cast<UserData::GenderType>(
                            atoi(reply->element[3]->str));
  data.face             = atoi(reply->element[4]->str);
  data.level            = atoi(reply->element[5]->str);
  data.exp              = atoi(reply->element[6]->str);
  data.scores           = atoi(reply->element[7]->str);
  data.coins            = atoi(reply->element[8]->str);
  data.win_count        = atoi(reply->element[9]->str);
  data.lost_count       = atoi(reply->element[10]->str);
  data.flee_count       = atoi(reply->element[11]->str);
  data.win_streak       = atoi(reply->element[12]->str);
  data.play_time        = atoi(reply->element[13]->str);
  data.login_count      = atoi(reply->element[14]->str);
  data.reg_time         = atoi(reply->element[15]->str);
  data.reg_addr         = atoi(reply->element[16]->str);
  data.last_login_time  = atoi(reply->element[17]->str);
  data.last_login_addr  = atoi(reply->element[18]->str);

  return true;
}

bool UserCache::Set(const std::string& account, const UserData& data) {
  util::SmartPtr<redisReply> reply(
      static_cast<redisReply*>(redisCommand(
          redis_.Get(), 
          "hmset %s " 
          "account %s "
          "user_id %u " 
          "user_name %s " 
          "gender %u " 
          "face %u " 
          "level %u " 
          "exp %u " 
          "scores %u " 
          "coins %u " 
          "win_count %u " 
          "lost_counta %u " 
          "flee_count %u " 
          "win_streak %u " 
          "play_time %u " 
          "login_count %u " 
          "reg_time %u " 
          "reg_addr %u " 
          "last_login_time %u "
          "last_login_addr %u", 
          data.account.c_str(), 
          data.user_id, 
          data.user_name.c_str(), 
          data.gender, 
          data.face, 
          data.level, 
          data.exp, 
          data.scores, 
          data.coins, 
          data.win_count, 
          data.lost_count, 
          data.flee_count, 
          data.win_streak, 
          data.play_time, 
          data.login_count, 
          data.reg_time, 
          data.reg_addr, 
          data.last_login_time, 
          data.last_login_addr)), 
      freeReplyObject);

  if (0 == reply->len)
    return false;
  if (0 != strcmp("OK", reply->str))
    return false;

  return true;
}

bool UserCache::Del(const std::string& account) {
  util::SmartPtr<redisReply> reply(
      static_cast<redisReply*>(redisCommand(
          redis_.Get(), "del %s", account.c_str())), 
      freeReplyObject);

  return (1 == reply->integer);
}
