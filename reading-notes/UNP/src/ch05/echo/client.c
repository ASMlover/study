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

static void logical_cli(FILE* stream, int sockfd) {
  char sendline[128];
  char recvline[128];

  while (fgets(sendline, sizeof(sendline), stream) != NULL) {
    common_writen(sockfd, sendline, strlen(sendline));

    if (common_readline(sockfd, sizeof(recvline), recvline) == 0)
      return;

    fprintf(stdout, "%s", recvline);
  }
}

int main(int argc, char* argv[]) {
  int sockfd;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  network_init();

  sockfd = common_socket(AF_INET, SOCK_STREAM, 0);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(5555);
  common_connect(sockfd, (struct sockaddr*)&addr, addrlen);

  logical_cli(stdin, sockfd);

  common_close(sockfd);
  network_destroy();

  return 0;
}
