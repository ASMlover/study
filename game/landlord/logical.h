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

  CARDTYPE_SINGLE,              // 单牌

  CARDTYPE_PAIR,                // 对牌
  CARDTYPE_THREE,               // 三张
  CARDTYPE_THREE_WITH_SINGLE,   // 三带一
  CARDTYPE_THREE_WITH_PAIR,     // 三带一对
  CARDTYPE_BOMB,                // 炸弹
  CARDTYPE_FOUR_WITH_TWOSINGLE, // 四带二
  CARDTYPE_FOUR_WITH_TWOPAIR,   // 四带二对

  CARDTYPE_STRAIGHT_SINGLE5,    // 单顺
  CARDTYPE_STRAIGHT_SINGLE6,    // 
  CARDTYPE_STRAIGHT_SINGLE7,    //
  CARDTYPE_STRAIGHT_SINGLE8,    //
  CARDTYPE_STRAIGHT_SINGLE9,    //
  CARDTYPE_STRAIGHT_SINGLE10,   //
  CARDTYPE_STRAIGHT_SINGLE11,   //
  CARDTYPE_STRAIGHT_SINGLE12,   //

  CARDTYPE_STRAIGHT_PAIR3,      // 双顺
  CARDTYPE_STRAIGHT_PAIR4,      //
  CARDTYPE_STRAIGHT_PAIR5,      //
  CARDTYPE_STRAIGHT_PAIR6,      //
  CARDTYPE_STRAIGHT_PAIR7,      //
  CARDTYPE_STRAIGHT_PAIR8,      //
  CARDTYPE_STRAIGHT_PAIR9,      //
  CARDTYPE_STRAIGHT_PAIR10,     //

  CARDTYPE_STRAIGHT_THREE2,     // 三顺
  CARDTYPE_STRAIGHT_THREE3,     //
  CARDTYPE_STRAIGHT_THREE4,     //
  CARDTYPE_STRAIGHT_THREE5,     //
  CARDTYPE_STRAIGHT_THREE6,     //
  CARDTYPE_STRAIGHT_THREE7,     //

  CARDTYPE_AIRPLANE_SINGLE2,    // 飞机带翅膀
  CARDTYPE_AIRPLANE_SINGLE3,    //
  CARDTYPE_AIRPLANE_SINGLE4,    //
  CARDTYPE_AIRPLANE_SINGLE5,    //
  CARDTYPE_AIRPLANE_PAIR2,      //
  CARDTYPE_AIRPLANE_PAIR3,      //
  CARDTYPE_AIRPLANE_PAIR4,      //
};


struct Card {
  uint8_t card;       // 牌
  uint8_t value;      // 牌值
};

class Logical : private UnCopyable {
  std::vector<Card> single_;
  std::vector<Card> pair_;
  std::vector<Card> three_;
  std::vector<Card> bomb_;
  std::vector<Card> rocket_;
public:
  Logical(void);
  ~Logical(void);

  bool PlayAnyCard(
      const std::vector<uint8_t>& cards, 
      std::vector<uint8_t>& out_cards);
  bool PlayCard(CardType type, 
      uint8_t value, 
      const std::vector<uint8_t>& cards, 
      std::vector<uint8_t>& out_cards);
private:
  bool CardsAnalysis(
      const std::vector<uint8_t>& cards, 
      std::map<uint8_t, std::vector<Card> >& cards_map);
  bool IsContinued(const Card* cards, int count, int step = 1);
  bool IsContinued(
      const std::pair<uint8_t, std::vector<Card> >* cards, 
      int count = 5);

  bool PlayAnySingle(std::vector<uint8_t>& out_cards);
  bool PlayAnyPair(std::vector<uint8_t>& out_cards);
  bool PlayAnyThree(std::vector<uint8_t>& out_cards);
  bool PlayAnyThreeWithSingle(std::vector<uint8_t>& out_cards);
  bool PlayAnyThreeWithPair(std::vector<uint8_t>& out_cards);
  bool PlayAnyBomb(std::vector<uint8_t>& out_cards);
  bool PlayRocket(std::vector<uint8_t>& out_cards);
  bool PlayStraight(
      std::map<uint8_t, std::vector<Card> >& cards, 
      std::vector<uint8_t>& out_cards);

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
