#ifndef __PLAYER_CACHE_HEADER_HPP__
#define __PLAYER_CACHE_HEADER_HPP__

struct PlayerData;
class PlayerCache : private UnCopyable {
  std::shared_ptr<redisContext> redis_;
public:
  explicit PlayerCache(void);
  ~PlayerCache(void);

  bool Init(const char* addr, int port);
  void Destroy(void);

  bool Get(const std::string& account, PlayerData& data);
  bool Set(const std::string& account, const PlayerData& data);
  bool Del(const std::string& account);
};

#endif  // __PLAYER_CACHE_HEADER_HPP__
