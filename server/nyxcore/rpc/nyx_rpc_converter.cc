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
#include <iostream>
#include "../utils/nyx_hex.h"
#include "../compressor/nyx_compressor.h"
#include "../crypter/nyx_crypter.h"
#include "nyx_rpc_converter.h"

namespace nyx { namespace rpc {

rpc_converter::rpc_converter(void) {
}

rpc_converter::~rpc_converter(void) {
}

void rpc_converter::handle_istream_data(
    const std::string& idata, std::string& odata) {
  if (decrypter_) {
    if (decrypter_->decrypt(idata, odata) <= 0) {
      std::cerr << "input data decrypt failed ..." << std::endl;
      return;
    }
  }
  if (compressor_) {
    if (decrypter_) {
      auto decrypted_data = odata;
      compressor_->decompress(decrypted_data, odata);
    }
    else {
      compressor_->decompress(idata, odata);
    }
  }
}

void rpc_converter::handle_ostream_data(
    const std::string& idata, std::string& odata) {
  if (compressor_)
    compressor_->compress(idata, odata);

  if (encrypter_) {
    int r = 0;
    if (compressor_) {
      auto compressed_data = odata;
      r = encrypter_->encrypt(compressed_data, odata);
    }
    else {
      r = encrypter_->encrypt(idata, odata);
    }

    if (r <= 0)
      std::cerr << "output data encrypt failed ..." << std::endl;
  }
}

}}
