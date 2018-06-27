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
  odata.resize(idata.size() + idata.size() / 1000 + MIN_BUFFER);

  auto out_len = static_cast<uInt>(odata.size());
  auto total_out = stream_.total_out;
  stream_.avail_in = static_cast<uInt>(idata.size());
  stream_.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(idata.data()));
  stream_.avail_out = out_len;
  stream_.next_out = reinterpret_cast<unsigned char*>(odata.data());

  auto err = deflate(&stream_, Z_NO_FLUSH);
  while (err == Z_OK && stream_.avail_out == 0) {
    odata.resize(odata.size() + DEFAULTALLOC);
    stream_.next_out = reinterpret_cast<unsigned char*>(
        odata.data() + stream_.total_out + total_out);
    out_len = DEFAULTALLOC;
    stream_.avail_out = out_len;
    err = deflate(&stream_, Z_NO_FLUSH);
  }
  if (err != Z_OK && err != Z_BUF_ERROR) {
    zlib_error(stream_, err, "while compressing");
    return err;
  }

  odata.resize(static_cast<std::size_t>(stream_.total_out - total_out));
  return err;
}

int zlib_compress_handler::flush(std::string& odata, int flushmode) {
  if (flushmode == Z_NO_FLUSH)
    return 0;

  auto old_size = odata.size();
  auto free_space = odata.capacity() - odata.size();
  if (free_space >= MIN_BUFFER)
    odata.resize(odata.capacity());
  else
    odata.resize(odata.size() + MIN_BUFFER);
  auto out_len = static_cast<uInt>(odata.size() - old_size);

  auto total_out = stream_.total_out;
  stream_.avail_in = 0;
  stream_.next_out = reinterpret_cast<unsigned char*>(odata.data()) + old_size;
  stream_.avail_out = out_len;

  auto err = deflate(&stream_, flushmode);
  while (err == Z_OK && stream_.avail_out == 0) {
    odata.resize(odata.size() + DEFAULTALLOC);
    stream_.next_out = reinterpret_cast<unsigned char*>(
        odata.data() + stream_.total_out - total_out + old_size);
    out_len = DEFAULTALLOC;
    stream_.avail_out = out_len;
    err = deflate(&stream_, flushmode);
  }
  if (err == Z_STREAM_END && flushmode == Z_FINISH) {
    if (err = deflateEnd(&stream_); err != Z_OK) {
      zlib_error(stream_, err, "from deflateEnd");
      return err;
    }
    initialized_ = false;
  }
  else if (err != Z_OK && err != Z_BUF_ERROR) {
    zlib_error(stream_, err, "while flushing");
    return err;
  }

  odata.resize(
      static_cast<std::size_t>(stream_.total_out - total_out) + old_size);
  return err;
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

zlib_compressor::zlib_compressor(int wbits, int memlevel)
  : compress_(new zlib_compress_handler())
  , decompress_(new zlib_decompress_handler()) {
  compress_->init(Z_DEFAULT_COMPRESSION, DEFLATED, wbits, memlevel, 0);
}

zlib_compressor::~zlib_compressor(void) {
}

void zlib_compressor::compress(const std::string& idata, std::string& odata) {
  if (idata.size() > 0) {
    compress_->compress(idata, odata);
    compress_->flush(odata, Z_SYNC_FLUSH);
  }
}

void zlib_compressor::decompress(const std::string& idata, std::string& odata) {
  if (idata.size() > 0)
    decompress_->decompress(idata, odata);
}

}}
