// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include "../TTypes.h"
#include "../TLogging.h"
#include "../TFileUtil.h"

typedef SSIZE_T ssize_t;

namespace tyr { namespace basic {

ReadSmallFile::ReadSmallFile(StringArg fname)
  : fd_(_open(fname.c_str(), _O_RDONLY)) {
  buffer_[0] = '\0';
  if (fd_ < 0)
    errno_ = errno;
}

ReadSmallFile::~ReadSmallFile(void) {
  if (fd_ >= 0)
    _close(fd_);
}

template <typename String>
int ReadSmallFile::read_to_string(int maxsz, String* content,
      int64_t* filesz, int64_t* modify_time, int64_t* create_time) {
  assert(nullptr != content);

  int err = errno_;
  if (fd_ > 0) {
    content->clear();

    if (nullptr != filesz) {
      struct _stat st;
      if (0 == _fstat(fd_, &st)) {
        if ((st.st_mode & _S_IFMT) == _S_IFREG) {
          *filesz = st.st_size;
          content->reserve(static_cast<int>(tyr_min(implicit_cast<int64_t>(maxsz), *filesz)));
        }
        else if ((st.st_mode & _S_IFMT) == _S_IFDIR) {
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
      size_t read_n = tyr_min(implicit_cast<size_t>(maxsz) - content->size(), sizeof(buffer_));
      ssize_t n = _read(fd_, buffer_, read_n);
      if (n > 0) {
        content->append(buffer_, n);
      }
      else {
        if (n < 0)
          err = errno;
        break;
      }
    }
  }

  return err;
}

int ReadSmallFile::read_to_buffer(int* size) {
  int err = errno_;
  if (fd_ >= 0) {
    _lseek(fd_, 0, SEEK_SET);
    ssize_t n = _read(fd_, buffer_, sizeof(buffer_) - 1);
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

template int ReadSmallFile::read_to_string(int, std::string*, int64_t*, int64_t*, int64_t*);
template int read_file(StringArg, int, std::string*, int64_t*, int64_t*, int64_t*);

}}
