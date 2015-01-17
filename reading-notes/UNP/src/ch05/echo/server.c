/*
 * Copyright (c) 2015 ASMlover. All rights reserved.
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
#include <common.h>

static DWORD WINAPI echo_process(void* arg) {
  int fd = *(int*)arg;

  ssize_t n;
  char buff[1024];

  while ((n = common_read(fd, sizeof(buff), buff)) > 0)
    common_writen(fd, buff, n);

  common_close(fd);

  return 0;
}

int main(int argc, char* argv[]) {
  int listenfd;
  int clientfd;
  struct sockaddr_in addr;
  socklen_t addrlen;

  network_init();

  listenfd = common_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(5555);
  common_bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
  common_listen(listenfd, SOMAXCONN);

  printf("echo server start sucess !!!\n");
  for (;;) {
    addrlen = sizeof(addr);
    clientfd = common_accept(listenfd, (struct sockaddr*)&addr, &addrlen);

    CreateThread(NULL, 0, echo_process, &clientfd, 0, NULL);
  }
  common_close(listenfd);

  network_destroy();

  return 0;
}
