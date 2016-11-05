// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <WinSock2.h>
#include <Windows.h>
#include <stdint.h>
#include <time.h>
#include <iostream>
#include <system_error>

#define UNUSED(x) ((void)x)

void __libnet_throw_error(int ec, const char* what) {
  throw std::system_error(std::error_code(ec, std::system_category()), what);
}

void __libnet_init(void) {
  WSADATA wd;
  int ec = WSAStartup(MAKEWORD(2, 2), &wd);
  if (0 != ec)
    __libnet_throw_error(ec, "winsock library startup failed");
}

void __libnet_destroy(void) {
  WSACleanup();
}

void server_main(const char* ip, uint16_t port) {
  SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == listener) {
    int ec = WSAGetLastError();
    __libnet_throw_error(ec, "create server socket failed");
  }

  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = inet_addr(ip);
  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(port);
  if (SOCKET_ERROR == bind(listener, (struct sockaddr*)&host_addr, sizeof(host_addr))) {
    int ec = WSAGetLastError();
    __libnet_throw_error(ec, "socket bind failed");
  }
  if (SOCKET_ERROR == listen(listener, SOMAXCONN)) {
    int ec = WSAGetLastError();
    __libnet_throw_error(ec, "socket listen failed");
  }

  std::cout << "server{" << ip << ":" << port << "} startup success ..." << std::endl;
  while (true) {
    struct sockaddr_in remote_addr;
    int addrlen = sizeof(remote_addr);
    SOCKET s = accept(listener, (struct sockaddr*)&remote_addr, &addrlen);
    std::cout << "accept remote client: "
      << s << ", from: " << inet_ntoa(remote_addr.sin_addr) << std::endl;

    time_t tick = time(nullptr);
    char buf[256];
    snprintf(buf, sizeof(buf), "current time: %.24s", ctime(&tick));
    WSABUF wbuf;
    wbuf.buf = buf;
    wbuf.len = strlen(buf);
    DWORD wbytes = 0;
    int rc = WSASend(s, &wbuf, 1, &wbytes, 0, nullptr, nullptr);
    if (SOCKET_ERROR == rc)
      break;

    shutdown(s, 2);
    closesocket(s);
  }
  shutdown(listener, 2);
  closesocket(listener);
}

void client_main(const char* ip, uint16_t port) {
  SOCKET connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == connector) {
    int ec = WSAGetLastError();
    __libnet_throw_error(ec, "create client socket failed");
  }

  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = inet_addr(ip);
  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(port);
  if (SOCKET_ERROR == connect(connector,
        (struct sockaddr*)&host_addr, sizeof(host_addr))) {
    int ec = WSAGetLastError();
    __libnet_throw_error(ec, "socket connect failed");
  }

  DWORD flags = 0, rbytes = 0;
  char buf[256] = {0};
  WSABUF wbuf;
  wbuf.len = sizeof(buf);
  wbuf.buf = buf;
  int ec = WSARecv(connector, &wbuf, 1, &rbytes, &flags, nullptr, nullptr);
  if (SOCKET_ERROR != ec)
    std::cout << "recv bytes: " << rbytes << ", recv data: " << buf << std::endl;

  shutdown(connector, 2);
  closesocket(connector);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "usage: ws.exe [options] ..." << std::endl;
    return 0;
  }

  __libnet_init();

  if (0 == strcmp(argv[1], "s"))
    server_main("127.0.0.1", 5555);
  else if (0 == strcmp(argv[1], "c"))
    client_main("127.0.0.1", 5555);

  __libnet_destroy();

  return 0;
}
