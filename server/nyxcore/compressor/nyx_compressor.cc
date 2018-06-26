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
#include "nyx_compressor.h"

namespace nyx { namespace compressor {

void zlib_error(
    const z_stream& zst, int err, const char* msg = "unknown zlib error") {
  auto zmsg = zst.msg;
  if (zmsg == Z_NULL) {
    switch (err) {
    case Z_VERSION_ERROR: zmsg = "zlib version mismatch"; break;
    case Z_MEM_ERROR: zmsg = "can not allocate memory for compress"; break;
    case Z_STREAM_ERROR: zmsg = "inconsistent stream state"; break;
    case Z_BUF_ERROR: zmsg = "incomplete or truncated stream"; break;
    case Z_DATA_ERROR: zmsg = "invalid input data"; break;
    }
  }
  if (zmsg == Z_NULL)
    std::cerr << __func__ << " : " << msg << std::endl;
  else
    std::cerr << __func__ << " : " << zmsg << std::endl;
}

zlib_compress_handler::zlib_compress_handler(void) {
}

zlib_compress_handler::~zlib_compress_handler(void) {
  if (initialized_)
    deflateEnd(&stream_);
}

bool zlib_compress_handler::init(
    int level, int method, int wbits, int memlevel, int strategy) {
  stream_.zalloc = static_cast<alloc_func>(Z_NULL);
  stream_.zfree = static_cast<free_func>(Z_NULL);
  stream_.next_in = nullptr;
  stream_.avail_in = 0;

  auto err = deflateInit2(&stream_, level, method, wbits, memlevel, strategy);
  if (err == Z_OK) {
    initialized_ = true;
    return true;
  }
  zlib_error(stream_, err);
  return false;
}

int zlib_compress_handler::compress(
    const std::string& idata, std::string& odata) {
  return 0;
}

int zlib_compress_handler::flush(std::string& odata, int flushmode) {
  return 0;
}

zlib_decompress_handler::zlib_decompress_handler(int wbits) {
  stream_.zalloc = static_cast<alloc_func>(Z_NULL);
  stream_.zfree = static_cast<free_func>(Z_NULL);
  stream_.next_in = nullptr;
  stream_.avail_in = 0;
  inflateInit2(&stream_, wbits);
}

zlib_decompress_handler::~zlib_decompress_handler(void) {
  if (initialized_)
    inflateEnd(&stream_);
}

int zlib_decompress_handler::_flush_unconsumed_input(int err) {
  return 0;
}

int zlib_decompress_handler::decompress(
    const std::string& idata, std::string& odata) {
  return 0;
}

int zlib_decompress_handler::flush(std::string& odata, int len) {
  return 0;
}

}}
