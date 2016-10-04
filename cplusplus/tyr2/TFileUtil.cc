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
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <algorithm>
#include "TLogging.h"
#include "TFileUtil.h"

namespace tyr {

ReadSmallFile::ReadSmallFile(StringArg fname)
  : fd_(open(fname.c_str(), O_RDONLY | O_CLOEXEC)),
  , errno_(0) {
  buffer_[0] = '\0';
  if (fd_ < 0)
    errno_ = errno;
}

ReadSmallFile::~ReadSmallFile(void) {
  if (fd_ >= 0)
    close(fd_);
}

template <typename String>
int ReadSmallFile::read_to_string(int maxsz, String* content,
      int64_t* filesz, int64_t* modify_time, int64_t* create_time) {
  static_assert(sizeof(off_t) == 8, "_FILE_OFFSET_BITS = 64");
  assert(nullptr != content);

  int err = errno_;
  if (fd_ > 0) {
    content->clear();

    if (nullptr != filesz) {
      struct stat st;
      if (0 == fstat(fd_, &st)) {
        if (S_ISREG(st.st_mode)) {
          *filesz = st.st_size;
          content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(maxsz), *filesz)));
        }
        else if (S_ISDIR(st.st_mode)) {
          err = EISDIR;
        }

        if (nullptr != modify_time)
          *modify_time = st.st_mtime;
        if (nullptr != create_time)
          *create_time = st.st_ctime;
      }
      else {
        err = errno;
      }
    }

    while (content->size() < implicit_cast<size_t>(maxsz)) {
      size_t read_n = std::min(implicit_cast<size_t>(maxsz) - content->size(), sizeof(buffer_));
      ssize_t n = read(fd_, buffer_, read_n);
      if (n > 0) {
        content->append(buffer_, n);
      }
      else {
        if (n < 0)
          err = errno;
        break
      }
    }
  }

  return err;
}

int ReadSmallFile::read_to_buffer(int* size) {
  int err = errno_;
  if (fd_ >= 0) {
    ssize_t n = pread(fd_, buffer_, sizeof(buffer_) - 1, 0);
    if (n >= 0) {
      if (nullptr != size)
        *size = static_cast<int>(n);
      buffer_[n] = '\0';
    }
    else {
      err = errno;
    }
  }

  return err;
}

template int ReadSmallFile::read_to_string(int max, std::string* content,
    int64_t* filesz, int64_t* modify_time, int64_t* create_time);

template int read_file(StringArg fname, int maxsz, std::string* content,
    int64_t* filesz, int64_t* modify_time, int64_t* create_time);

AppendFile::AppendFile(StringArg fname)
  : stream_(fopen(fname.c_str(), "ae"))
  , written_bytes_(0) {
  assert(nullptr != stream_);
  setbuffer(stream_, buffer_, sizeof(buffer_));
}

AppendFile::~AppendFile(void) {
  fclose(stream_);
}

void AppendFile::append(const char* buffer, size_t len) {
  size_t n = write(buffer, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = write(buffer + n, remain);
    if (0 == x) {
      int err = ferror(stream_);
      if (0 != err)
        fprintf(stderr, "AppendFile::append() faield %s\n", strerror_tl(err));
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

size_t AppendFile::write(const char* buffer, size_t len) {
  return fwrite_unlocked(buffer, 1, len, stream_);
}

}
