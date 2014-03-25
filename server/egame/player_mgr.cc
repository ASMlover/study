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
#include "player_mgr.h"



PlayerMgr::PlayerMgr(void) {
}

PlayerMgr::~PlayerMgr(void) {
}


Player* PlayerMgr::GetPlayerByConnID(uint32_t connid) {
  if (static_cast<uint32_t>(INVAL_CONNECTOR) == connid)
    return NULL;

  std::map<uint32_t, Player*>::iterator it = player_cache_.find(connid);
  if (it != player_cache_.end())
    return it->second;

  return NULL;
}

Player* PlayerMgr::GetPlayerByID(uint32_t id) {
  if (UserData::INVAL_USERID == id)
    return NULL;

  std::map<uint32_t, Player*>::iterator it = player_list_.find(id);
  if (it != player_list_.end())
    return it->second;

  return NULL;
}


bool PlayerMgr::Dispatch(
    uint32_t connid, const void* data, uint32_t size) {
  return false;
}
