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
#include <errno.h>
#include <string.h>
#include "rio.h"

#define RIO_BUFSIZE (8192)

typedef struct rio_t {
  int rio_fd;
  int rio_cnt;
  char* rio_bufptr;
  char rio_buf[RIO_BUFSIZE];
} rio_t;

ssize_t rio_readn(int fd, void* usrbuf, size_t n) {
  size_t nleft = n;
  ssize_t nread;
  char* bufp = usrbuf;

  while (nleft > 0) {
    if ((nread = read(fd, bufp, nleft)) < 0) {
      if (EINTR == errno)
        nread = 0;
      else 
        return -1;
    }
    else if (0 == nread) {
      break;
    }
    nleft -= nread;
    bufp += nread;
  }

  return (n - nleft);
}

ssize_t rio_writen(int fd, void* usrbuf, size_t n) {
  size_t nleft = n;
  ssize_t nwritten;
  char* bufp = usrbuf;

  while (nleft > 0) {
    if ((nwritten = write(fd, bufp, nleft)) <= 0) {
      if (EINTR == errno)
        nwritten = 0;
      else
        return -1;
    }
    nleft -= nwritten;
    bufp += nwritten;
  }

  return n;
}

void rio_readinitb(rio_t* rp, int fd) {
  rp->rio_fd = fd;
  rp->rio_cnt = 0;
  rp->rio_bufptr = rp->rio_buf;
}

static ssize_t rio_read(rio_t* rp, char* usrbuf, size_t n) {
  int cnt;

  while (rp->rio_cnt <= 0) {
    rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
    if (rp->rio_cnt < 0) {
      if (EINTR == errno)
        return -1;
    }
    else if (0 == rp->rio_cnt) {
      return 0;
    }
    else {
      rp->rio_bufptr = rp->rio_buf;
    }
  }

  cnt = n;
  if (rp->rio_cnt < n)
    cnt = rp->rio_cnt;
  memcpy(usrbuf, rp->rio_bufptr, cnt);
  rp->rio_bufptr += cnt;
  rp->rio_cnt -= cnt;

  return cnt;
}

ssize_t rio_readlineb(rio_t* rp, void* usrbuf, size_t maxlen) {
  int n, rc;
  char c;
  char* bufp = usrbuf;

  for (n = 1; n < maxlen; ++n) {
    if (1 == (rc = rio_read(rp, &c, 1))) {
      *bufp++ = c;
      if ('\n' == c)
        break;
    }
    else if (0 == rc) {
      if (1 == n)
        return 0;
      else
        break;
    }
    else {
      return -1;
    }
  }

  *bufp = 0;

  return n;
}

ssize_t rio_readnb(rio_t* rp, void* usrbuf, size_t n) {
  size_t nleft = 0;
  ssize_t nread;
  char* bufp = usrbuf;

  while (nleft > 0) {
    if ((nread = rio_read(rp, bufp, nleft)) < 0) {
      if (EINTR == errno)
        nread = 0;
      else
        return -1;
    }
    else if (0 == nread) {
      break;
    }

    nleft -= nread;
    bufp += nread;
  }

  return (n - nleft);
}
