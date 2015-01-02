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

#if defined(PLATFORM_WIN)
void common_close(int fd) {
  shutdown(fd, SD_BOTH);
  closesocket(fd);
}

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

ssize_t common_readn(int fd, size_t nbytes, void* buff) {
  size_t nleft = nbytes;
  ssize_t nread;
  char* ptr = (char*)buff;
  
  while (nleft > 0) {
    if ((nread = recv(fd, ptr, nleft, 0)) < 0) {
      if (EINTR == errno)
        nread = 0;
      else
        return -1;
    }
    else if (0 == nread) {
      break;
    }

    nleft -= nread;
    ptr += nread;
  }

  return (nbytes - nleft);
}

ssize_t common_writen(int fd, const void* buff, size_t nbytes) {
  size_t nleft = nbytes;
  ssize_t nwrite;
  const char* ptr = (const char*)buff;

  while (nleft > 0) {
    if ((nwrite = send(fd, ptr, nleft, 0)) <= 0) {
      if (nwrite < 0 && EINTR == errno)
        nwrite = 0;
      else
        return -1;
    }

    nleft -= nwrite;
    ptr += nwrite;
  }

  return nbytes;
}

ssize_t common_readline(int fd, size_t maxlen, void* buff) {
  ssize_t n, rc;
  char c;
  char* ptr = (char*)buff;

  for (n = 1; n < (ssize_t)maxlen; ++n) {
  again:
    if (1 == (rc = recv(fd, &c, 1, 0))) {
      *ptr++ = c;
      if ('\n' == c)
        break;
    }
    else if (0 == rc) {
      *ptr = 0;
      return (n - 1);
    }
    else {
      if (EINTR == errno)
        goto again;
      return -1;
    }
  }
  *ptr = 0;

  return n;
}
#else
void common_close(int fd) {
  shutdown(fd, SHUT_RDWR);
  close(fd);
}

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

ssize_t common_readn(int fd, size_t nbytes, void* buff) {
  /* body */
  return 0;
}

ssize_t common_writen(int fd, const void* buff, size_t nbytes) {
  /* body */
  return 0;
}

ssize_t common_readline(int fd, size_t maxlen, void* buff) {
  /* body */
  return 0;
}
#endif
