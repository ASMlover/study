// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include "../tyr.h"
#include "tyr_base64.h"

namespace tyr {

Base64::Base64(void) {
  InitDecode64();
}

void Base64::InitDecode64(void) {
  kEncode64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
  memset(kDecode64, 0, sizeof(kDecode64));
  for (auto i = 0; i < 64; ++i)
    kDecode64[static_cast<int>(kEncode64[i])] = i;
}

std::string Base64::Encode(const std::string& s) {
  std::string r;

  const byte_t* b = reinterpret_cast<const byte_t*>(s.c_str());
  int n = static_cast<int>(s.size());
  for (auto i = 0; i < n; i += 3) {
    r.push_back(kEncode64[b[i] >> 2]);

    if (i + 1 < n) {
      r.push_back(kEncode64[((b[i] & 0x03) << 4) | (b[i + 1]) >> 4]);
    }
    else {
      r.push_back(kEncode64[(b[i] & 0x03) << 4]);
      r.append("==");
      break;
    }

    if (i + 2 < n) {
      r.push_back(kEncode64[((b[i + 1] & 0x0f) << 2) | (b[i + 2] >> 6)]);
      r.push_back(kEncode64[b[i + 2] & 0x3f]);
    }
    else {
      r.push_back(kEncode64[(b[i + 1] & 0x0f) << 2]);
      r.push_back('=');
      break;
    }
  }

  return r;
}

std::string Base64::Decode(const std::string& s) {
  std::string r;

  const byte_t* b = reinterpret_cast<const byte_t*>(s.c_str());
  int n = static_cast<int>(s.size());
  for (auto i = 0; i < n; i += 4) {
    r.push_back((kDecode64[b[i]] << 2) | ((kDecode64[b[i + 1]] >> 4) & 0x03));
    if (b[i + 2] != '=')
      r.push_back((kDecode64[b[i + 1]] << 4) | ((kDecode64[b[i + 2]] >> 2) & 0x0f));
    if (b[i + 3] != '=')
      r.push_back((kDecode64[b[i + 2]] << 6) | (kDecode64[b[i + 3]] & 0x3f));
  }

  return r;
}

}
