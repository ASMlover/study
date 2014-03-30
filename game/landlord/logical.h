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
#ifndef __LOGICAL_HEADER_H__
#define __LOGICAL_HEADER_H__

enum CardType {
  CARDTYPE_UNKNOWN = 0, 
  CARDTYPE_SINGLE,    // 单牌

  CARDTYPE_PAIR,      // 对牌
};


struct Card {
  uint8_t card;       // 牌
  uint8_t value;      // 牌值
};

class Logical : private UnCopyable {
  std::vector<uint8_t>& cards_;

  std::vector<Card> single_;
  std::vector<Card> pair_;
  std::vector<Card> three_;
  std::vector<Card> bomb_;
  std::vector<Card> rocket_;
public:
  explicit Logical(std::vector<uint8_t>& cards);
  ~Logical(void);

  bool PlayAnyCard(std::vector<uint8_t>& out_cards);
  bool PlayCard(CardType type, std::vector<uint8_t>& out_cards);
private:
  bool CardsAnalysis(void);
  bool IsContinued(const Card* cards, int count, int step = 1);

  bool PlayAnySingle(std::vector<uint8_t>& out_cards);
  bool PlayAnyPair(std::vector<uint8_t>& out_cards);
  bool PlayAnyThree(std::vector<uint8_t>& out_cards);
  bool PlayAnyThreeWithSingle(std::vector<uint8_t>& out_cards);
  bool PlayAnyThreeWithPair(std::vector<uint8_t>& out_cards);
  bool PlayAnyBomb(std::vector<uint8_t>& out_cards);
  bool PlayRocket(std::vector<uint8_t>& out_cards);

  bool PlaySingle(uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayPair(uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayThree(uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayThreeWithSingle(uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayThreeWithPair(uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayBomb(uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayFourWithTwoSingle(
      uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayFourWithTwoPair(
      uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayStraightSingle(int num, 
      uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayStraightPair(int num, 
      uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayStraightThree(int num, 
      uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayAirplaneWithSingle(int num, 
      uint8_t value, std::vector<uint8_t>& out_cards);
  bool PlayAirplaneWithPair(int num, 
      uint8_t value, std::vector<uint8_t>& out_cards);
};

#endif  //! __LOGICAL_HEADER_H__
