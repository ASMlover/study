// Copyright (c) 2024 ASMlover. All rights reserved.
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
#include "util.hh"

#if !defined(SDB_WINDOWS)
# include <unistd.h>
#endif

namespace sdb {

#if defined(SDB_WINDOWS)
int open(const char* filename, int oflag, int pmode) {
  return ::_open(filename, oflag, pmode);
}

int close(int fd) {
  return ::_close(fd);
}

ssz_t read(const int fd, void* buffer, const u32_t buffer_size) {
  return ::_read(fd, buffer, buffer_size);
}

ssz_t write(int fd, const void* buffer, u32_t count) {
  return ::_write(fd, buffer, count);
}

off_t lseek(int fd, off_t offset, int origin) {
  return ::_lseek(fd, offset, origin);
}
#else
int open(const char* filename, int oflag, int pmode) {
  return ::open(filename, oflag, pmode);
}

int close(int fd) {
  return ::close(fd);
}

ssz_t read(const int fd, void* buffer, const u32_t buffer_size) {
  return ::read(fd, buffer, buffer_size);
}

ssz_t write(int fd, const void* buffer, u32_t count) {
  return ::write(fd, buffer, cout);
}

off_t lseek(int fd, off_t offset, int origin) {
  return ::lseek(fd, offset, origin);
}
#endif

}
