/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
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
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_EPOLL   (256)
#define MAX_EVENTS  (32)



static void 
setnonblocking(int s)
{
  int opt = fcntl(s, F_GETFL);

  if (opt < 0) {
    fprintf(stderr, "fcntl(s, F_GETFL)");
    exit(1);
  }

  opt |= O_NONBLOCK;
  if (fcntl(s, F_SETFL, opt) < 0) {
    fprintf(stderr, "fcntl(s, F_SETFL, opt)");
    exit(1);
  }
}




int 
main(int argc, char* argv[]) 
{
  int srv_fd, clt_fd, fd, epoll_fd;
  int i, nfds, sa_len;
  struct epoll_event ev, events[MAX_EVENTS];
  struct sockaddr_in srv_addr, clt_addr;
  char buf[128];

  epoll_fd = epoll_create(MAX_EPOLL);
  srv_fd = socket(AF_INET, SOCK_STREAM, 0);
  setnonblocking(srv_fd);

  ev.data.fd  = srv_fd;
  ev.events   = EPOLLIN | EPOLLET;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, srv_fd, &ev);

  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
  srv_addr.sin_family       = AF_INET;
  srv_addr.sin_port         = htons(5555);
  bind(srv_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
  listen(srv_fd, 20);

  fprintf(stdout, "server init success ...\n");
  for ( ; ; ) {
    nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

    for (i = 0; i < nfds; ++i) {
      if (events[i].data.fd == srv_fd) {
        sa_len = sizeof(clt_addr);
        fprintf(stdout, "begin to accept client connection ...\n");
        clt_fd = accept(srv_fd, (struct sockaddr*)&clt_addr, &sa_len);
        if (clt_fd < 0) {
          fprintf(stderr, "accept error ...");
          exit(1);
        }

        setnonblocking(clt_fd);
        ev.data.fd  = clt_fd;
        ev.events   = EPOLLIN | EPOLLET;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clt_fd, &ev);
      }
      else if (events[i].events & EPOLLIN) {
        int ret;

        memset(buf, 0, sizeof(buf));
        if ((fd = events[i].data.fd) < 0)
          continue;

        ret = recv(fd, buf, 128, 0);
        if (ret < 0) {
          if (ECONNRESET == errno) {
            close(fd);
            events[i].data.fd = -1;
          }
          else 
            fprintf(stderr, "recv data error");
        }
        else if (0 == ret) {
          close(fd);
          events[i].data.fd = -1;
        }

        fprintf(stdout, "recv buffer: %s\n", buf);

        ev.data.fd  = fd;
        ev.events   = EPOLLOUT | EPOLLET;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
      }
      else if (events[i].events & EPOLLOUT) {
        fd = events[i].data.fd;

        send(fd, buf, strlen(buf), 0);

        ev.data.fd  = fd;
        ev.events   = EPOLLIN | EPOLLET;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
      }
    }
  }

  close(srv_fd);
  close(epoll_fd);

  return 0;
}
