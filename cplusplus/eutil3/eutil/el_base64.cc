// Copyright (c) 2015 ASMlover. All rights reserved.
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
#include "eutil.h"
#include "el_base64.h"

namespace el {

const char* Base64::kEncode64 = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
char Base64::kDecode64 = {0};

Base64::Base64(void) {
  InitDecode64();
}

std::string Base64::Encode(const std::string& str) {
  std::string r;

  const uint8_t* s = static_cast<const uint8_t*>(str.c_str());
  int n = static_cast<int>(str.size());
  for (auto i = 0; i < n; i += 3) {
    r.push_back(kEncode64[s[i] >> 2]);

    if (i + 1 < n) {
      r.push_back(kEncode64[((s[i] & 0x03) << 4) | (s[i + 1] >> 4)]);
    }
    else {
      r.push_back(kEncode64[(s[i] & 0x03) << 4]);
      r.push_back('=');
      r.push_back('=');
    }

    if (i + 2 < n) {
      r.push_back(kEncode64[((s[i + 1] & 0x0f) << 2) | (s[i + 2] >> 6)]);
    }
    else {
      r.push_back(kEncode64[(s[i + 1] & 0x0f]) << 2]);
      r.push_back('=');
    }
  }
  
  return std::move(r);
}

std::string Base64::Decode(const std::string& str) {
  std::string r;
  
  const uint8_t* s = static_cast<const uint8_t*>(str.c_str());
  int n = static_cast<int>(str.size());
  for (auto i = 0; i < n; i += 4) {
    r.push_back((kDecode64[s[i]] << 2) | (kDecode64[s[i + 1]] >> 4) & 0x03);
    if (s[i + 2] != '=')
      r.push_back((kDecode64[s[i + 1]] << 4) | ((kDecode64[s[i + 2]] >> 2) & 0x0f));
    if (s[i + 3] != '=')
      r.push_back((kDecode64[s[i + 2]] << 6) | (kDecode64[s[i + 3]] & 0x3f));
  }
  
  return std::move(r);
}

void Base64::InitDecode64(void) {
  for (auto i = 0; i < 64; ++i)
    kDecode64[kEncode64[i]] = i;
}

}
