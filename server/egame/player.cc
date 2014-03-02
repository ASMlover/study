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
#include "player.h"



Player::Player(void) {
  ResetPlayer();
}

Player::~Player(void) {
}

void Player::ResetPlayer(void) {
  state_  = PLAYERSTATE_OFFLINE;
  connid_ = INVAL_CONNECTOR;

  data_.account         = "";
  data_.user_id         = GameData::INVAL_USERID;
  data_.user_name       = "";
  data_.gender          = GameData::GENDERTYPE_UNKNOWN;
  data_.face            = GameData::DEFAULT_FACEID;
  data_.level           = 0;
  data_.exp             = 0;
  data_.scores          = 0;
  data_.coins           = 0;
  data_.win_count       = 0;
  data_.lost_count      = 0;
  data_.flee_count      = 0;
  data_.win_streak      = 0;
  data_.play_time       = 0;
  data_.lost_count      = 0;
  data_.reg_time        = 0;
  data_.reg_addr        = 0;
  data_.last_login_time = 0;
  data_.last_login_addr = 0;
}
