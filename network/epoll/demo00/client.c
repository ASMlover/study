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
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>




int 
main(int argc, char* argv[])
{
  int i;
  int pid;
  int fd;
  char buf[128];
  struct sockaddr_in sa;


  pid = (int)getpid();
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == fd) {
    fprintf(stderr, "create socket error...");
    return -1;
  }
  sa.sin_addr.s_addr = inet_addr("127.0.0.1");
  sa.sin_family      = AF_INET;
  sa.sin_port        = htons(5555);
  fprintf(stdout, "begin to connect to server ...\n");
  if (0 != connect(fd, (struct sockaddr*)&sa, sizeof(sa))) {
    fprintf(stderr, "connect to server failed ...\n");
    return -1;
  }

  fprintf(stdout, "begin to send message to server ...\n");
  for (i = 0; i < 100; ++i) {
    sprintf(buf, "[%d]this is send message > %d", pid, i + 1);
    send(fd, buf, strlen(buf), 0);

    memset(buf, 0, sizeof(buf));
    recv(fd, buf, sizeof(buf), 0);
    fprintf(stdout, "recv: %s\n", buf);
  }

  close(fd);

  return 0;
}
