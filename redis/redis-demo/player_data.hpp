#ifndef __PLAYER_DATA_HEADR_HPP__
#define __PLAYER_DATA_HEADR_HPP__

struct PlayerData {
  id_t        id;
  std::string account;
  std::string name;
  id_t        face;
  int         coins;
  int         scores;
};

#endif  // __PLAYER_DATA_HEADR_HPP__
