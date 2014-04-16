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
