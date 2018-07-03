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
#include "nyx_crypter.h"

namespace nyx { namespace crypter {

int base_crypter::encrypt(const char* idata, std::size_t size, char* odata) {
  return algorithm_->encrypt(idata, size, odata);
}

int base_crypter::decrypt(const char* idata, std::size_t size, char* odata) {
  return algorithm_->decrypt(idata, size, odata);
}

int base_crypter::encrypt(const std::string& idata, std::string& odata) {
  return algorithm_->encrypt(idata, odata);
}

int base_crypter::decrypt(const std::string& idata, std::string& odata) {
  return algorithm_->decrypt(idata, odata);
}

key_crypter::key_crypter(const std::string& keypath)
  : base_crypter(new algorithm_rsa(keypath)) {
}

rc4_crypter::rc4_crypter(const std::string& key)
  : base_crypter(new algorithm_rc4(key)) {
}

}}
