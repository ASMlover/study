// Copyright (c) 2014 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

enum class ConstValue {
  CONSTVALUE_MULTIPLIER = 31, 
  CONSTVALUE_NHASH = 4093, 
};

uint32_t getHash(const char* s) {
  uint32_t hash = 0;
  uint8_t* p;
  for (p = (uint8_t*)s; *p != '\0', ++p)
    hash = CONSTVALUE_MULTIPLIER * hash + *p;

  return hash % CONSTVALUE_NHASH;
}



// MS(System.String) HASH
uint32_t msHash(const char* s, int len) {
  uint32_t num1 = 0x15051505;
  uint32_t num2 = num1;
  uint32_t* ptr = (uint32_t*)s;

  for (int i = len; i > 0; i -= 4) {
    num1 = (((num1 << 5) + num1) + (num1 >> 0x1b)) ^ ptr[0];
    if (i <= 2)
      break;

    num2 = (((num2 << 5) + num2) + (num2 >> 0x1b)) ^ ptr[1];
    ptr += 2;
  }

  return (num1 + (num2 * 0x5d588b65));
}

// RS(Robert Sedgwicks) HASH 
uint32_t rsHash(const String& s) {
  uint32_t a = 63689, b = 378551;
  uint32_t hash = 0;
  for (auto c : s) {
    hash = hash * a + c;
    a *= b;
  }

  return hash;
}

// JS(Justin Sobel) HASH 
uint32_t jsHash(const String& s) {
  uint32_t hash = 1315423911;
  for (auto c : s) 
    hash ^= ((hash << 5) + c + (hash >> 2));

  return hash;
}

// PJW HASH 
uint32_t pjwHash(const String& s) {
  uint32_t bitsInUint = 32;
  uint32_t threeQuarters = (bitsInUint * 3) / 4;
  uint32_t oneEighth = (bitsInUint / 8);
  uint32_t highBits = (0xffffffff) << (bitsInUint - oneEighth);
  uint32_t hash = 0, test = 0;

  for (auto c : s) {
    hash = (hash << oneEighth) + c;
    if ((test = hash & highBits) != 0)
      hash = ((hash ^ (test >> threeQuarters)) & (~highBits));
  }

  return hash;
}

// ELF HASH (used in unix)
uint32_t elfHash(const String& s) {
  uint32_t hash = 0;
  uint32_t x = 0;
  for (auto c : s) {
    hash = (hash << 4) + c;
    if ((x = hash & 0xf0000000) != 0)
      hash ^= (x >> 24);
    hash &= ~x;
  }

  return hash;
}

// BKDR(Brian Kernighan & Dennis Ritchie) HASH 
uint32_t bkdrHash(const String& s) {
  uint32_t seed = 131;  // 131 1313 13131 131313 1313131 13131313 etc
  uint32_t hash = 0;
  for (auto c : s) 
    hash = (hash * seed) + c;

  return hash;
}

// SDBM HASH 
uint32_t sdbmHash(const String& s) {
  uint32_t hash = 0;
  for (auto c : s) 
    hash = c + (hash << 6) + (hash << 16) - hash;

  return hash;
}

// DJB(Daniel J.Bernstein) HASH 
uint32_t djbHash(const String& s) {
  uint32_t hash = 5381;
  for (auto c : s) 
    hash = ((hash << 5) + hash) + c;

  return hash;
}

// DEK(Knuth) HASH 
uint32_t dekHash(const String& s) {
  uint32_t hash = s.length();
  for (auto c : s) 
    hash = ((hash << 5) ^ (hash >> 27)) ^ c;

  return hash;
}

// AP(Arash Partow) HASH 
uint32_t apHash(const String& s) {
  uint32_t hash = 0xaaaaaaaa;
  for (int i = 0; i < s.length(); ++i) {
    if ((i & 1) == 0)
      hash ^= ((hash << 7) ^ s[i] * (hash >> 3));
    else 
      hash ^= (~((hash << 11) + s[i] ^ (hash >> 5)));
  }

  return hash;
}
