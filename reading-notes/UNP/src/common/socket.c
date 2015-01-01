/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "common.h"

int common_socket(int family, int type, int protocol) {
  int fd = socket(family, type, protocol);
  if (fd < 0)
    error_print("socket error\n");

  return fd;
}

void common_bind(int fd, struct sockaddr* addr, socklen_t addrlen) {
  if (bind(fd, addr, addrlen) < 0)
    error_print("bind error\n");
}

void common_listen(int fd, int backlog) {
  if (listen(fd, backlog) < 0)
    error_print("listen error\n");
}

int common_accept(int fd, struct sockaddr* addr, socklen_t* addrlen) {
  int s = accept(fd, addr, addrlen);
  if (s < 0)
    error_print("accept error\n");

  return s;
}

void common_connect(int fd, struct sockaddr* addr, socklen_t addrlen) {
  if (connect(fd, addr, addrlen) < 0)
    error_print("connect error\n");
}

int common_read(int fd, int buflen, char* buffer) {
  int read_size = recv(fd, buffer, buflen, 0);
  if (read_size < 0)
    error_print("read error\n");

  return read_size;
}

int common_write(int fd, const char* buffer, int buflen) {
  int write_size = send(fd, buffer, buflen, 0);
  if (write_size < 0)
    error_print("write error\n");

  return write_size;
}

void common_close(int fd) {
#if defined(PLATFORM_WIN)
  shutdown(fd, SD_BOTH);
  closesocket(fd);
#else
  shutdown(fd, SHUT_RDWR);
  close(fd);
#endif
}

#if defined(PLATFORM_WIN)
char* common_ntop(const struct sockaddr* addr, socklen_t addrlen) {
  static char str[128];

  switch (addr->sa_family) {
  case AF_INET: 
    {
      struct sockaddr_in* sa = (struct sockaddr_in*)addr;
      char* strptr = NULL;
      if (NULL == (strptr = inet_ntoa(sa->sin_addr)))
        return NULL;
      if (0 != ntohs(sa->sin_port))
        snprintf(str, sizeof(str), "%s:%d", strptr, ntohs(sa->sin_port));
    }
    break;
  }

  return str;
}
#else
char* common_ntop(const struct sockaddr* addr, socklen_t addrlen) {
  static char str[128];

  switch (addr->sa_family) {
  case AF_INET: 
    {
      struct sockaddr_in* sa = (struct sockaddr_in*)addr;
      if (NULL == inet_ntop(AF_INET, &sa->sin_addr, str, sizeof(str)))
        return NULL;
      if (0 != ntohs(sa->sin_port))
        snprintf(str, sizeof(str), "%s:%d", str, ntohs(sa->sin_port));
    }
    break;
  }

  return str;
}
#endif
