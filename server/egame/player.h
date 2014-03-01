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
#ifndef __PLAYER_HEADER_H__
#define __PLAYER_HEADER_H__

// record player's game data every turn 
// in this game; user_id is the global 
// unique ID for every player, and account 
// is also global uniqued
struct GameData {
  enum GenderType {
    GENDERTYPE_UNKNOWN  = 0, 
    GENDERTYPE_MALE     = 1,    // male 
    GENDERTYPE_FEMALE   = 2,    // female
  };

  std::string account;          // player account info
  uint32_t    user_id;          // player ID
  std::string user_name;        // player name in game
  GenderType  gender;           // player's gender in game
  uint16_t    face;             // player's face ID 
  uint16_t    level;            // player's level in game 
  uint64_t    exp;              // player's experience
  uint32_t    scores;           // player's scores in game 
  uint32_t    coins;            // player's coins int game
  uint32_t    win_count;        // count of player win
  uint32_t    lost_count;       // count of player lost
  uint32_t    flee_count;       // count of player flee 
  uint32_t    win_streak;       // player winning-streak count
  time_t      play_time;        // play time about this game
  uint32_t    login_count;      // total times about enter game 
  time_t      reg_time;         // time about player register this game 
  uint32_t    reg_addr;         // address about player register this game
  time_t      last_login_time;  // time about player last login game 
  uint32_t    last_login_addr;  // address about player last login game
};


enum PlayerState {
  PLAYERSTATE_OFFLINE         = 0, 
  PLAYERSTATE_CONNECT         = 1, 
  PLAYERSTATE_LOGIN           = 2, 
  PLAYERSTATE_IN_MATCH_QUEUE  = 3, 
  PLAYERSTATE_IN_GAME_ROOM    = 4, 
  PLAYERSTATE_EXIT_GAME_ROOM  = 5, 
  PLAYERSTATE_LOGOFF          = 6, 
  PLAYERSTATE_DISCONNECT      = 7, 
};

class Player : UnCopyable {
  PlayerState state_;
  GameData    data_;
public:
  explicit Player(void);
  ~Player(void);

  inline const GameData& GetData(void) const {
    return data_;
  }
private:
  void ResetPlayer(void);
};

#endif  //! __PLAYER_HEADER_H__
