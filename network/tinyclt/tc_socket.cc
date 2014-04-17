// Copyright (c) 2014 ASMlover. All rights reserved.
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
#include "tc_header.h"
#include "tc_socket.h"



Socket::Socket(void) 
  : fd_(NETTYPE_INVAL) {
}

Socket::~Socket(void) {
  Close();
}

bool Socket::Open(void) {
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (NETTYPE_INVAL == fd_)
    return false;

  return true;
}

void Socket::Close(void) {
  if (NETTYPE_INVAL != fd_) {
#if defined(USE_WINDOWS)
    shutdown(fd_, SD_BOTH);
    closesocket(fd_);
#elif defined(USE_POSIX)
    shutdown(fd_, SHUT_RDWR);
    close(fd_);
#endif

    fd_ = NETTYPE_INVAL;
  }
}

bool Socket::Connect(const char* address, uint16_t port) {
  if (NETTYPE_INVAL == fd_)
    return false;

  if (nullptr == address) 
    address = "127.0.0.1";
  struct sockaddr_in remote_addr;
  remote_addr.sin_addr.s_addr = inet_addr(address);
  remote_addr.sin_family      = AF_INET;
  remote_addr.sin_port        = htons(port);
  if (NETTYPE_ERR == connect(fd_, 
        (struct sockaddr*)&remote_addr, sizeof(remote_addr)))
    return false;

  return true;
}

int Socket::Read(int bytes, char* buffer) {
  if (NETTYPE_INVAL == fd_ || nullptr == buffer)
    return NETTYPE_ERR;

  return recv(fd_, buffer, bytes, 0);
}

int Socket::Write(const char* buffer, int bytes) {
  if (NETTYPE_INVAL == fd_ || nullptr == buffer)
    return NETTYPE_ERR;

  int write_bytes = 0;
  int ret;
  while (write_bytes < bytes) {
    ret = send(fd_, buffer + write_bytes, bytes - write_bytes, 0);
    if (NETTYPE_ERR == ret)
      return ret;

    write_bytes += ret;
  }

  return write_bytes;
}
