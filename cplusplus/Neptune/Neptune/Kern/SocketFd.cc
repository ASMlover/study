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
#include <Neptune/Kern/SocketFd.h>

namespace Neptune {

bool SocketFd::open(sa_family_t family) {
  // TODO:
  return false;
}

void SocketFd::shutdown_read(void) {
  // TODO:
}
void SocketFd::shutdown_write(void) {
  // TODO:
}
void SocketFd::close(void) {
  // TODO:
}
bool SocketFd::bind(const struct sockaddr* addr) {
  // TODO:
  return false;
}

bool SocketFd::listen(void) {
  // TODO:
  return false;
}

int SocketFd::accept(struct sockaddr* addr) {
  // TODO:
  return 0;
}

bool SocketFd::connect(const struct sockaddr* addr) {
  // TODO:
  return false;
}

ssize_t SocketFd::read(std::size_t len, void* buf) {
  // TODO:
  return 0;
}

ssize_t SocketFd::write(const void* buf, std::size_t len) {
  // TODO:
  return 0;
}

void SocketFd::set_iovec(Iovec_t& vec, char* buf, size_t len) {
  // TODO:
}

ssize_t SocketFd::readv(int niov, Iovec_t* iov) {
  // TODO:
  return false;
}

void SocketFd::set_nonblock(void) {
  // TODO:
}

void SocketFd::set_option(int level, int optname, int optval) {
  // TODO:
}

std::string SocketFd::to_string(const struct sockaddr* addr, bool /*ip_only*/) {
  // TODO:
  return "";
}

std::string SocketFd::to_string(const struct sockaddr* addr) {
  // TODO:
  return "";
}

void SocketFd::get_address(const char* ip, std::uint16_t port, struct sockaddr_in* addr) {
  // TODO:
}

void SocketFd::get_address(const char* ip, std::uint16_t port, struct sockaddr_in6* addr) {
  // TODO:
}

int SocketFd::get_error(void) {
  // TODO:
  return 0;
}

struct sockaddr_in6 SocketFd::get_local(void) const {
  struct sockaddr_in6 addr{};
  // TODO:
  return addr;
}

struct sockaddr_in6 SocketFd::get_peer(void) const {
  struct sockaddr_in6 addr{};
  // TODO:
  return addr;
}

bool SocketFd::is_self_connect(void) const {
  // TODO:
  return false;
}

struct sockaddr* SocketFd::cast(struct sockaddr_in* addr) {
  // TODO:
  return nullptr;
}

const struct sockaddr* SocketFd::cast(const struct sockaddr_in* addr) {
  // TODO:
  return nullptr;
}

struct sockaddr* SocketFd::cast(struct sockaddr_in6* addr) {
  // TODO:
  return nullptr;
}

const struct sockaddr* SocketFd::cast(const struct sockaddr_in6* addr) {
  // TODO:
  return nullptr;
}

const struct sockaddr_in* SocketFd::to_v4(const struct sockaddr* addr) {
  // TODO:
  return nullptr;
}

const struct sockaddr_in6* SocketFd::to_v6(const struct sockaddr* addr) {
  // TODO:
  return nullptr;
}

}
