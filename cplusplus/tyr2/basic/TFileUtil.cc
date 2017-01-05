// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "TConfig.h"
#include "TLogging.h"
#include "TFileUtil.h"

namespace tyr { namespace basic {

AppendFile::AppendFile(StringArg fname)
  : stream_(fopen(fname.c_str(), "ae"))
  , written_bytes_(0) {
  assert(nullptr != stream_);
  setvbuf(stream_, buffer_, _IOFBF, sizeof(buffer_));
}

AppendFile::~AppendFile(void) {
  fclose(stream_);
}

void AppendFile::append(const char* buf, size_t len) {
  size_t n = write(buf, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = write(buf + n, remain);
    if (0 == x) {
      int err = ferror(stream_);
      if (0 != err)
        fprintf(stderr, "AppendFile::append - faield %s\n", strerror_tl(err));
      break;
    }
    n += x;
    remain = len - n;
  }

  written_bytes_ += len;
}

void AppendFile::flush(void) {
  fflush(stream_);
}

size_t AppendFile::write(const char* buf, size_t len) {
#if defined(TYR_WINDOWS)
  return _fwrite_nolock(buf, 1, len, stream_);
#else
  return fwrite_unlocked(buf, 1, len, stream_);
#endif
}

}}
