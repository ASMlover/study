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
#pragma once

#include <string>
#include <memory>
#include <boost/noncopyable.hpp>
#include <openssl/rc4.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

namespace nyx { namespace crypter {

class base_encrypt_algorithm : private boost::noncopyable {
public:
  virtual ~base_encrypt_algorithm(void) {}
  virtual int encrypt(const char* idata, std::size_t size, char* odata) = 0;
  virtual int decrypt(const char* idata, std::size_t size, char* odata) = 0;
  virtual int encrypt(const std::string& idata, std::string& odata) = 0;
  virtual int decrypt(const std::string& idata, std::string& odata) = 0;
};

class algorithm_rc4 : public base_encrypt_algorithm {
  std::unique_ptr<RC4_KEY> key_rc4_;

  virtual bool _import_key(const std::string& key);
public:
  algorithm_rc4(const std::string& key);

  virtual int encrypt(const char* idata, std::size_t size, char* odata) override;
  virtual int decrypt(const char* idata, std::size_t size, char* odata) override;
  virtual int encrypt(const std::string& idata, std::string& odata) override;
  virtual int decrypt(const std::string& idata, std::string& odata) override;
};

class algorithm_rsa : public base_encrypt_algorithm {
  std::size_t keylen_{};
  RSA* key_rsa_{};

  virtual bool _import_keypath(const std::string& keypath);
public:
  algorithm_rsa(const std::string& keypath);
  virtual ~algorithm_rsa(void);

  virtual int encrypt(const char* idata, std::size_t size, char* odata) override;
  virtual int decrypt(const char* idata, std::size_t size, char* odata) override;
  virtual int encrypt(const std::string& idata, std::string& odata) override;
  virtual int decrypt(const std::string& idata, std::string& odata) override;
};

class algorithm_sha : private boost::noncopyable {
public:
  algorithm_sha(void) {}
  virtual ~algorithm_sha(void) {}

  static std::string digest(const std::string& seed);
};

}}
