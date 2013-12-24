//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include "el_net_internal.h"
#include "el_socket.h"
#include "el_connector.h"




namespace el {


Connector::Connector(void)
  : events_(0)
  , spinlock_()
  , writable_(true)
{
}

Connector::~Connector(void)
{
  rbuf_.Destroy();
  wbuf_.Destroy();
}


int 
Connector::Read(uint32_t bytes, char* buffer)
{
  if (0 == bytes || NULL == buffer)
    return kNetTypeError;

  return rbuf_.Get(bytes, buffer);
}

int 
Connector::Write(const char* buffer, uint32_t bytes)
{
  if (NULL == buffer || 0 == bytes)
    return kNetTypeError;

  int ret;
  {
    LockerGuard<SpinLock> guard(spinlock_);
    ret = wbuf_.Put(buffer, bytes);
  }

  if (writable_)
    DealWithAsyncWrite();

  return ret;
}

bool 
Connector::WriteBufferEmpty(void)
{
  bool ret;

  {
    LockerGuard<SpinLock> guard(spinlock_);
    ret = (0 == wbuf_.length());
  }

  return ret;
}

void 
Connector::SetWritable(bool writable)
{
  writable_ = writable;
}



int 
Connector::DealWithAsyncRead(void)
{
  int read_bytes = 0;
  int ret;

  while (true) {
    char* free_buffer = rbuf_.free_buffer();
    uint32_t free_length = rbuf_.free_length();

    if (0 == free_length) {
      if (!rbuf_.Regrow())
        return kNetTypeError;

      free_buffer = rbuf_.free_buffer();
      free_length = rbuf_.free_length();
    }

    ret = Recv(free_length, free_buffer);

    if (ret > 0) {
      rbuf_.Increment(ret);
      read_bytes += ret;
      if ((uint32_t)ret < free_length)
        break;
    }
    else if (0 == ret) {
      if (read_bytes > 0)
        break;
      else 
        return ret;
    }
    else {
      return ret;
    }
  }

  return read_bytes;
}

int 
Connector::DealWithAsyncWrite(void)
{
  int write_bytes = 0;
  int ret;

  LockerGuard<SpinLock> guard(spinlock_);
  while (true) {
    uint32_t length = wbuf_.length();
    if (0 == length) {
      if (write_bytes > 0)
        break;
      else 
        return kNetTypeError;
    }

    const char* buffer = wbuf_.buffer();
    ret = Send(buffer, length);

    if (ret > 0) {
      wbuf_.Decrement(ret);
      write_bytes += ret;
      if ((uint32_t)ret < length)
        break;
    }
    else if (0 == ret) {
      if (write_bytes > 0)
        break;
      else 
        return ret;
    }
    else {
      return ret;
    }
  }

  return write_bytes;
}



}
