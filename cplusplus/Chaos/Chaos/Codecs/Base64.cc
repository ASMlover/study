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
#include <string.h>
#include <Chaos/Types.h>
#include <Chaos/Codecs/Base64.h>

namespace Chaos {

namespace Base64 {
  static const char* base64_initilize(void);

  static char kDecode64[257] = {};
  static const char* kEncode64 = base64_initilize();

  static const char* base64_initilize(void) {
    const char* s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    for (int i = 0; i < 64; ++i)
      kDecode64[static_cast<int>(s[i])] = i;
    return s;
  }

  std::string encode(const char* s, size_t n) {
    std::string r;

    const byte_t* b = reinterpret_cast<const byte_t*>(s);
    for (size_t i = 0; i < n; i += 3) {
      r.push_back(kEncode64[b[i] >> 2]);

      if (i + 1 < n) {
        r.push_back(kEncode64[((b[i] & 0x03) << 4) | (b[i + 1] >> 4)]);
      }
      else {
        r.push_back(kEncode64[(b[i] & 0x03) << 4]);
        r.push_back('=');
        r.push_back('=');
      }

      if (i + 2 < n) {
        r.push_back(kEncode64[((b[i + 1] & 0x0f) << 2) | (b[i + 2] >> 6)]);
      }
      else {
        r.push_back(kEncode64[(b[i + 1] & 0x0f) << 2]);
        r.push_back('=');
      }
    }

    return r;
  }

  std::string decode(const char* s, size_t n) {
    std::string r;

    const byte_t* b = reinterpret_cast<const byte_t*>(s);
    for (size_t i = 0; i < n; i += 4) {
      r.push_back((kDecode64[b[i]] << 2) | ((kDecode64[b[i + 1]] >> 4) & 0x03));
      if (b[i + 2] != '=')
        r.push_back((kDecode64[b[i + 1]] << 4) | ((kDecode64[b[i + 2]] >> 2) & 0x0f));
      if (b[i + 3] != '=')
        r.push_back((kDecode64[b[i + 2]] << 6) | (kDecode64[b[i + 3]] & 0x3f));
    }

    return r;
  }

  std::string encode(const char* s) {
    return encode(s, strlen(s));
  }

  std::string decode(const char* s) {
    return decode(s, strlen(s));
  }

  std::string encode(const std::string& s) {
    return encode(s.data(), s.size());
  }

  std::string decode(const std::string& s) {
    return decode(s.data(), s.size());
  }
}

}
