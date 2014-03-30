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
#include "logical.h"


#define CARDVALUE_SMALL_KING  (13)
#define CARDVALUE_BIG_KING    (14)
#define CARDVALUE_2POINT      (12)


Logical::Logical(std::vector<uint8_t>& cards) 
  : cards_(cards) {
}

Logical::~Logical(void) {
}

bool Logical::PlayAnyCard(std::vector<uint8_t>& out_cards) {
  if (!CardsAnalysis())
    return false;

  return PlayAnySingle(out_cards) 
    || PlayAnyPair(out_cards) 
    || PlayAnyThree(out_cards) 
    || PlayAnyThreeWithSingle(out_cards) 
    || PlayAnyThreeWithPair(out_cards) 
    || PlayAnyBomb(out_cards);
}

bool Logical::PlayCard(CardType type, std::vector<uint8_t>& out_cards) {
  return true;
}




bool Logical::CardsAnalysis(void) {
  if (cards_.empty())
    return false;

  // card value => cards 
  std::map<uint8_t, std::vector<Card> > src_cards;
  Card c;
  int  n = static_cast<int>(cards_.size());
  for (int i = 0; i < n; ++i) {
    c.card = cards_[i];
    c.value = CardValue(cards_[i]);

    src_cards[c.value].push_back(c);
  }

  std::map<uint8_t, std::vector<Card> >::iterator it;
  for (it = src_cards.begin(); it != src_cards.end(); ++it) {
    switch (it->second.size()) {
    case 1:
      single_.push_back(it->second[0]);
      break;
    case 2:
      if (CARDVALUE_SMALL_KING == it->second[0].value
          && CARDVALUE_BIG_KING == it->second[1].value) {
        bomb_.push_back(it->second[0]);
        bomb_.push_back(it->second[1]);
      }
      else {
        pair_.push_back(it->second[0]);
        pair_.push_back(it->second[1]);
      }
      break;
    case 3:
      three_.push_back(it->second[0]);
      three_.push_back(it->second[1]);
      three_.push_back(it->second[2]);
      break;
    case 4:
      bomb_.push_back(it->second[0]);
      bomb_.push_back(it->second[1]);
      bomb_.push_back(it->second[2]);
      bomb_.push_back(it->second[3]);
      break;
    default:
      return false;
    }
  }

  return true;
}

bool Logical::IsContinued(const std::vector<Card>& cards, int step) {
  int n = static_cast<int>(cards.size());
  for (int i = 0; i < n - step; i += step) {
    if (cards[i].value + 1 != cards[i + step].value)
      return false;
  }

  return true;
}


bool Logical::PlayAnySingle(std::vector<uint8_t>& out_cards) {
  if (single_.empty())
    return false;

  out_cards.clear();
  out_cards.push_back(single_[0].card);

  return true;
}

bool Logical::PlayAnyPair(std::vector<uint8_t>& out_cards) {
  if (pair_.size() < 2)
    return false;

  out_cards.clear();
  out_cards.push_back(pair_[0].card);
  out_cards.push_back(pair_[1].card);

  return true;
}

bool Logical::PlayAnyThree(std::vector<uint8_t>& out_cards) {
  if (three_.size() < 3)
    return false;

  out_cards.clear();
  out_cards.push_back(three_[0].card);
  out_cards.push_back(three_[1].card);
  out_cards.push_back(three_[2].card);

  return true;
}

bool Logical::PlayAnyThreeWithSingle(std::vector<uint8_t>& out_cards) {
  if (single_.empty() || three_.size() < 3)
    return false;

  out_cards.clear();
  out_cards.push_back(three_[0].card);
  out_cards.push_back(three_[1].card);
  out_cards.push_back(three_[2].card);
  out_cards.push_back(single_[0].card);

  return true;
}

bool Logical::PlayAnyThreeWithPair(std::vector<uint8_t>& out_cards) {
  if (pair_.size() < 2 || three_.size() < 3)
    return false;
  
  out_cards.clear();
  out_cards.push_back(three_[0].card);
  out_cards.push_back(three_[1].card);
  out_cards.push_back(three_[2].card);
  out_cards.push_back(pair_[0].card);
  out_cards.push_back(pair_[1].card);

  return true;
}

bool Logical::PlayAnyBomb(std::vector<uint8_t>& out_cards) {
  if (bomb_.empty())
    return false;

  if (CARDVALUE_SMALL_KING == bomb_[0].value) {
    if (bomb_.size() < 2)
      return false;

    out_cards.push_back(bomb_[0].card);
    out_cards.push_back(bomb_[1].card);
  }
  else {
    if (bomb_.size() < 4) 
      return false;

    out_cards.push_back(bomb_[0].card);
    out_cards.push_back(bomb_[1].card);
    out_cards.push_back(bomb_[2].card);
    out_cards.push_back(bomb_[3].card);
  }

  return true;
}
