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

#include <sstream>
#include <memory>
#include <zlib.h>
#include <boost/noncopyable.hpp>

#define DEFLATED 8
#if MAX_MEM_LEVEL >= 8
# define DEF_MEM_LEVEL 8
#else
# define DEF_MEM_LEVEL MAX_MEM_LEVEL
#endif
#define DEF_WBITS MAX_WBITS

#define DEFAULTALLOC 1024
#define MIN_BUFFER 256
#define ZIP_RATE 2

namespace nyx { namespace compressor {

class base_compressor : private boost::noncopyable {
public:
  virtual ~base_compressor(void) {}
  virtual void compress(const std::string& idata, std::string& odata) = 0;
  virtual void decompress(const std::string& idata, std::string& odata) = 0;
};

class zlib_compress_handler : private boost::noncopyable {
  bool initialized_{};
  z_stream stream_;
public:
  zlib_compress_handler(void);
  ~zlib_compress_handler(void);

  bool init(int level = Z_DEFAULT_COMPRESSION, int method = DEFLATED,
      int wbits = MAX_WBITS, int memlevel = DEF_MEM_LEVEL, int strategy = 0);
  int compress(const std::string& idata, std::string& odata);
  int flush(std::string& odata, int flushmode = Z_FINISH);
};

class zlib_decompress_handler : private boost::noncopyable {
  bool initialized_{true};
  std::string unused_{};
  std::string unconsumed_{};
  z_stream stream_;

  int _flush_unconsumed_input(int err);
public:
  zlib_decompress_handler(int wbits = MAX_WBITS);
  ~zlib_decompress_handler(void);

  int decompress(const std::string& idata, std::string& odata);
};

class zlib_compressor : public base_compressor {
  std::unique_ptr<zlib_compress_handler> compress_;
  std::unique_ptr<zlib_decompress_handler> decompress_;
public:
  zlib_compressor(int wbits = MAX_WBITS, int memlevel = DEF_MEM_LEVEL);
  virtual ~zlib_compressor(void);

  virtual void compress(const std::string& idata, std::string& odata) override;
  virtual void decompress(const std::string& idata, std::string& odata) override;
};

}}
