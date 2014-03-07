#include "global.h"
#include "player_data.h"
#include "player_cache.h"


PlayerCache::PlayerCache(void)
  : redis_(static_cast<redisContext*>(NULL), redisFree) {
}

PlayerCache::PlayerCache(void) {
}

bool PlayerCache::Init(const char* addr, int port) {
  struct timeval tv = {1, 500000};
  redis_ = std::shared_ptr<redisContext>(
      redisConnectWithTimeout(addr, port, tv), 
      redisFree);

  if (NULL == redis_.get() || 0 != redis_->err) {
    fprintf(stderr, "Connect to redis %s:%d FAILED\n", addr, port);
    return false;
  }

  return true;
}

void PlayerCache::Destroy(void) {
}


bool PlayerCache::Get(const std::string& account, PlayerData& data) {
  std::shared_ptr<redisReply> reply(
      static_cast<redisReply*>(redisCommand(
          redis_.get(), "exists %s", account.c_str())), 
      freeReplyObject);
  if (NULL == reply.get())
    return false;

  if (REDIS_REPLY_INTEGER != reply->type)
    return false;
  if (0 == reply->integer)
    return false;

  reply = std::shared_ptr<redisReply>(
      static_cast<redisReply*>(redisCommand(
          redis_.get(), 
          "hmget %s id account name face coins scores", 
          account.c_str())), 
      freeReplyObject);
  if (REDIS_REPLY_ARRAY != reply->type)
    return false;

  data.id       = atoi(reply->element[0]->str);
  data.account  = reply->element[1]->str;
  data.name     = reply->element[2]->str;
  data.face     = atoi(reply->element[3]->str);
  data.coins    = atoi(reply->element[4]->str);
  data.scores   = atoi(reply->element[5]->str);

  return true;
}

bool PlayerCache::Set(const std::string& account, const PlayerData& data) {
  std::shared_ptr<redisReply> reply(
      static_cast<redisReply*>(redisCommand(
          redis_.get(), 
          "hmset %s id %u account %s name %s " 
          "face %u coins %d scores %d", 
          account.c_str(), 
          data.id, data.account.c_str(), data.name.c_str(), 
          data.face, data.coins, data.scores)), 
      freeReplyObject);

  if (0 == reply->len)
    return false;

  if (0 != strcmp("OK", reply->str))
    return false;

  return true;
}

bool PlayerCache::Del(const std::string& account) {
  std::shared_ptr<redisReply> reply(
      static_cast<redisReply*>(redisCommand(
          redis_.get(), "del %s", account.c_str())), 
      freeReplyObject);

  return (1 == reply->integer);
}
