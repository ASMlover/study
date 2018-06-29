// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "nyx_encrypt_algorithm.h"

namespace nyx { namespace encrypter {

algorithm_rc4::algorithm_rc4(const std::string& key) {
  _import_key(key);
}

algorithm_rc4::~algorithm_rc4(void) {
  if (key_rc4_ != nullptr)
    delete key_rc4_;
}

bool algorithm_rc4::_import_key(const std::string& key) {
  key_rc4_ = new RC4_KEY();
  RC4_set_key(key_rc4_,
      key.size(), reinterpret_cast<const unsigned char*>(key.data()));
  return true;
}

int algorithm_rc4::encrypt(const char* idata, std::size_t size, char* odata) {
  if (key_rc4_ == nullptr)
    return -1;
  RC4(key_rc4_,
      size,
      reinterpret_cast<const unsigned char*>(idata),
      reinterpret_cast<unsigned char*>(odata));
  return size;
}

int algorithm_rc4::decrypt(const char* idata, std::size_t size, char* odata) {
  return encrypt(idata, size, odata);
}

int algorithm_rc4::encrypt(const std::string& idata, std::string& odata) {
  if (odata.size() < idata.size())
    odata.resize(idata.size());
  return encrypt(idata.data(), idata.size(), const_cast<char*>(odata.data()));
}

int algorithm_rc4::decrypt(const std::string& idata, std::string& odata) {
  if (odata.size() < idata.size())
    odata.resize(idata.size());
  return decrypt(idata.data(), idata.size(), const_cast<char*>(odata.data()));
}

std::string algorithm_sha::digest(const std::string& seed) {
  SHA_CTX c;
  unsigned char md[21]{};
  SHA1_Init(&c);
  SHA1_Update(&c, seed.c_str(), seed.size());
  SHA1_Final(md, &c);
  return std::string(reinterpret_cast<const char*>(md));
}

}}
