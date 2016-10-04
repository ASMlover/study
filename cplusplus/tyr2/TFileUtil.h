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
#ifndef __TYR_FILEUTIL_HEADER_H__
#define __TYR_FILEUTIL_HEADER_H__

#include "TUnCopyable.h"
#include "TStringPiece.h"

namespace tyr {

class ReadSmallFile : private UnCopyable {
  static const int kBufferSize = 64*1024;
  int fd_;
  int errno_;
  char buffer_[kBufferSize];
public:
  explicit ReadSmallFile(StringArg fname);
  ~ReadSmallFile(void);

  template <typename String>
  int read_to_string(int maxsz, String* content,
      int64_t* filesz, int64_t* modify_time, int64_t* create_time);
  int read_to_buffer(int* size);

  const char* buffer(void) const {
    return buffer_;
  }
};

template <typename String>
int read_file(StringArg fname, int maxsz, String* content,
    int64_t* filesz = nullptr, int64_t* modify_time = nullptr, int64_t* create_time = nullptr) {
  ReadSmallFile file(fname);
  return file.read_to_string(maxsz, content, filesz, modify_time, create_time);
}

class AppendFile : private UnCopyable {
  FILE* stream_;
  char buffer_[64*1024];
  size_t written_bytes_;

  size_t write(const char* buffer, size_t len);
public:
  explicit AppendFile(StringArg fname);
  ~AppendFile(void);

  void append(const char* buffer, size_t len);
  void flush(void);

  size_t written_bytes(void) const {
    return written_bytes_;
  }
};

}

#endif // __TYR_FILEUTIL_HEADER_H__
