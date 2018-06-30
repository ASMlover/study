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
#include <cstdio>
#include <memory>
#include <openssl/pem.h>
#include "nyx_encrypt_algorithm.h"

namespace nyx { namespace crypter {

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

algorithm_rsa::algorithm_rsa(const std::string& keypath) {
  _import_keypath(keypath);
}

algorithm_rsa::~algorithm_rsa(void) {
  if (key_rsa_ != nullptr)
    RSA_free(key_rsa_);
}

bool algorithm_rsa::_import_keypath(const std::string& keypath) {
  std::unique_ptr<std::FILE, int (*)(std::FILE*)> fp(
      std::fopen(keypath.c_str(), "r"), &std::fclose);

  if (!fp)
    return false;
  if (key_rsa_ = PEM_read_RSAPrivateKey(fp.get(), nullptr, nullptr, nullptr);
      key_rsa_ == nullptr)
    return false;
  keylen_ = RSA_size(key_rsa_);

  return true;
}

int algorithm_rsa::encrypt(const char* idata, std::size_t size, char* odata) {
  if (key_rsa_ == nullptr)
    return -1;

  return RSA_public_encrypt(size,
      reinterpret_cast<unsigned char*>(const_cast<char*>(idata)),
      reinterpret_cast<unsigned char*>(odata), key_rsa_, RSA_PKCS1_OAEP_PADDING);
}

int algorithm_rsa::decrypt(const char* idata, std::size_t size, char* odata) {
  if (key_rsa_ == nullptr)
    return -1;

  return RSA_private_decrypt(keylen_,
      reinterpret_cast<unsigned char*>(const_cast<char*>(idata)),
      reinterpret_cast<unsigned char*>(odata), key_rsa_, RSA_PKCS1_OAEP_PADDING);
}

int algorithm_rsa::encrypt(const std::string& idata, std::string& odata) {
  if (odata.size() < keylen_)
    odata.resize(keylen_);

  auto r = encrypt(idata.data(), idata.size(), const_cast<char*>(odata.data()));
  if (r > 0)
    odata.resize(r);
  return r;
}

int algorithm_rsa::decrypt(const std::string& idata, std::string& odata) {
  if (odata.size() < keylen_)
    odata.resize(keylen_);

  auto r = decrypt(idata.data(), idata.size(), const_cast<char*>(odata.data()));
  if (r > 0)
    odata.resize(r);
  return r;
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
