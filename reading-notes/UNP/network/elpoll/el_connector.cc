// Copyright (c) 2015 ASMlover. All rights reserved.
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
#include "el_poll.h"
#include "el_buffer.h"
#include "el_socket.h"
#include "el_connector.h"

namespace el {

Connector::Connector(void)
  : locker_()
  , writable_(true) {
}

Connector::~Connector(void) {
  rbuf_.Destroy();
  wbuf_.Destroy();
}

int Connector::Read(uint32_t bytes, char* buffer) {
  if (0 == bytes || nullptr == buffer)
    return EL_NETERR;

  return static_cast<int>(rbuf_.Get(bytes, buffer));
}

int Connector::Write(const char* buffer, uint32_t bytes) {
  if (nullptr == buffer || 0 == bytes)
    return EL_NETERR;

  int ret; 
  {
    LockerGuard<SpinLock> guard(locker_);
    ret = static_cast<int>(wbuf_.Put(buffer, bytes));
  }

  if (writable_)
    AsyncWriter();

  return ret;
}

int Connector::AsyncReader(void) {
  int read_bytes = 0;

  while (true) {
    char* free_buffer = rbuf_.free_buffer();
    uint32_t free_length = rbuf_.free_length();

    if (0 == free_length) {
      if (!rbuf_.Regrow())
        return EL_NETERR;

      free_buffer = rbuf_.free_buffer();
      free_length = rbuf_.free_length();
    }

    int ret = Get(free_length, free_buffer);
    if (ret > 0) {
      rbuf_.Inc(ret);
      read_bytes += ret;
      if (static_cast<uint32_t>(ret) < free_length)
        break;
    }
    else if (0 == ret && read_bytes > 0) {
      break;
    }
    else {
      return EL_NETERR;
    }
  }

  return read_bytes;
}

int Connector::AsyncWriter(void) {
  int write_bytes = 0;

  LockerGuard<SpinLock> guard(locker_);
  while (true) {
    uint32_t length = wbuf_.length();
    if (0 == length) {
      if (write_bytes > 0)
        break;
      else
        return EL_NETERR;
    }

    int ret = Put(wbuf_.buffer(), length);
    if (ret > 0) {
      wbuf_.Dec(ret);
      write_bytes += ret;
      if (static_cast<uint32_t>(ret) < length)
        break;
    }
    else if (0 == ret && write_bytes > 0) {
      break;
    }
    else {
      return EL_NETERR;
    }
  }

  return write_bytes;
}

}
