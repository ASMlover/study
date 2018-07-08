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

#include <memory>
#include <string>
#include <boost/noncopyable.hpp>

namespace nyx { namespace crypter {

class base_encrypt_algorithm;

class base_crypter : private boost::noncopyable {
  std::unique_ptr<base_encrypt_algorithm> algorithm_;
public:
  explicit base_crypter(base_encrypt_algorithm* algorithm);
  virtual ~base_crypter(void);

  virtual int encrypt(const char* idata, std::size_t size, char* odata);
  virtual int decrypt(const char* idata, std::size_t size, char* odata);
  virtual int encrypt(const std::string& idata, std::string& odata);
  virtual int decrypt(const std::string& idata, std::string& odata);
};

class key_crypter : public base_crypter {
public:
  explicit key_crypter(const std::string& keypath);
};

class rc4_crypter : public base_crypter {
public:
  explicit rc4_crypter(const std::string& key);
};

}}
