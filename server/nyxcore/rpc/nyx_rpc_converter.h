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

namespace nyx {

class base_crypter;
class base_compressor;

using base_crypter_ptr = std::shared_ptr<base_crypter>;
using base_compressor_ptr = std::shared_ptr<base_compressor>;

namespace rpc {

class rpc_converter : private boost::noncopyable {
  base_crypter_ptr encrypter_;
  base_crypter_ptr decrypter_;
  base_compressor_ptr compressor_;
public:
  rpc_converter(void);
  ~rpc_converter(void);

  void handle_istream_data(const std::string& data, std::string& output);
  void handle_ostream_data(const std::string& data, std::string& output);

  void set_crypter(
      const base_crypter_ptr& encrypter, const base_crypter_ptr& decrypter) {
    encrypter_ = encrypter;
    decrypter_ = decrypter_;
  }

  void set_compressor(const base_compressor_ptr& compressor) {
    compressor_ = compressor;
  }

  void reset_crypter(void) {
    encrypter_.reset();
    decrypter_.reset();
  }

  void reset_compressor(void) {
    compressor_.reset();
  }
};

}}
