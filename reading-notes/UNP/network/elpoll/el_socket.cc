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
#include "el_address.h"
#include "el_socket.h"

namespace el {

Socket::Socket(void)
  : fd_(-1) {
}

Socket::~Socket(void) {
  Close();
}

bool Socket::SetOption(int level, int optname, int optval) {
  return true;
}

bool Socket::SetNonBlock(void) {
  return true;
}

bool Socket::SetTcpNoDelay(bool nodelay) {
  return true;
}

bool Socket::SetReuseAddr(bool reuse) {
  return true;
}

bool Socket::SetKeepAlive(bool alive) {
  return true;
}

bool Socket::Open(void) {
  return true;
}

void Socket::Close(void) {
}

bool Socket::Bind(const char* addr, uint16_t port) {
  return true;
}

bool Socket::Listen(void) {
  return true;
}

bool Socket::Accept(Socket& connector, Address& addr) {
  return true;
}

bool Socket::Connect(const char* addr, uint16_t port) {
  return true;
}

int Socket::Get(int bytes, char* buffer) {
  return 0;
}

int Socket::Put(const char* buffer, int bytes) {
  return 0;
}

}
