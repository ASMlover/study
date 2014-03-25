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


#include "user_data.h"


// player's state in game server
enum PlayerState {
  PLAYERSTATE_OFFLINE         = 0,
  PLAYERSTATE_LINK            = 1, 
  PLAYERSTATE_LOGIN           = 2, 
  PLAYERSTATE_IN_MATCH_QUEUE  = 3, 
  PLAYERSTATE_IN_GAME_ROOM    = 4, 
  PLAYERSTATE_EXIT_GAME_ROOM  = 5, 
  PLAYERSTATE_LOGOFF          = 6, 
  PLAYERSTATE_BREAK           = 7, 
};

class Player : private util::UnCopyable {
  uint32_t    connid_;
  PlayerState state_;
  UserData    data_;
public:
  explicit Player(uint32_t connid);
  ~Player(void);
public:
  // get player's data information, 
  // we can't modified the result data information
  inline const UserData& GetData(void) const {
    return data_;
  }

  inline void SetData(const UserData& data) {
    data_ = data;
  }

  // get player's current state
  inline const PlayerState GetState(void) const {
    return state_;
  }

  inline void SetState(PlayerState state) {
    state_ = state;
  }

  // get player's connector identifier
  inline uint32_t GetConnID(void) const {
    return connid_;
  }
private:
  void ResetPlayer(void);
};

#endif  //! __PLAYER_HEADER_H__
