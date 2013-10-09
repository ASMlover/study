//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include <winsock2.h>
#include <vector>
#include "global.h"


class Server {
  SOCKET fd_;
  bool   running_;
  HANDLE thread_;
  HANDLE start_event_;
  bool (*read_routine_)(SOCKET, void*);
  fd_set rset_;
  std::vector<SOCKET> fds_;

  Server(const Server&);
  Server& operator =(const Server&);
public:
  explicit Server(void);
  ~Server(void);

  void Start(const char* ip, unsigned short port);
  void Stop(void);
  void Run(bool (*read_routine)(SOCKET, void*));
private:
  static DWORD WINAPI Worker(void* arg);
};



Server::Server(void)
  : fd_(INVALID_SOCKET)
  , running_(false)
  , thread_(NULL)
  , start_event_(NULL)
  , read_routine_(NULL)
{
  FD_ZERO(&rset_);
}

Server::~Server(void)
{
  Stop();
}

void 
Server::Start(const char* ip, unsigned short port)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == fd_) {
    LOG_ERR("socket failed ...\n");
    abort();
  }

  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = inet_addr(ip);
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);
  if (SOCKET_ERROR == bind(fd_, 
      (struct sockaddr*)&host_addr, sizeof(host_addr))) {
    LOG_ERR("bind failed ...\n");
    abort();
  }

  if (SOCKET_ERROR == listen(fd_, SOMAXCONN)) {
    LOG_ERR("listen failed ...\n");
    abort();
  }
}

void 
Server::Stop(void)
{
  running_ = false;

  std::vector<SOCKET>::iterator it;
  for (it = fds_.begin(); it != fds_.end(); ++it) {
    shutdown(*it, SD_BOTH);
    closesocket(*it);
    *it = INVALID_SOCKET;
  }

  if (NULL != thread_) {
    WaitForSingleObject(thread_, INFINITE);

    CloseHandle(thread_);
    thread_ = NULL;
  }

  closesocket(fd_);
}

void 
Server::Run(bool (*read_routine)(SOCKET, void*))
{
  read_routine_ = read_routine;

  start_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (NULL == start_event_) {
    LOG_ERR("CreateEvent failed ...\n");
    abort();
  }

  running_ = true;
  thread_ = CreateThread(NULL, 0, &Server::Worker, this, 0, NULL);
  if (NULL != thread_)
    WaitForSingleObject(start_event_, INFINITE);
  CloseHandle(start_event_);

  fprintf(stdout, "server running ...\n");
  while (running_) {
    struct sockaddr_in remote_addr;
    int addrlen = sizeof(remote_addr);
    SOCKET s = accept(fd_, (struct sockaddr*)&remote_addr, &addrlen);
    if (INVALID_SOCKET == s) {
      Sleep(1);
      continue;
    }

    fprintf(stdout, "===> get connection from [%s] [%d]\n",
        inet_ntoa(remote_addr.sin_addr), s);
    fds_.push_back(s);
  }
}



DWORD WINAPI 
Server::Worker(void* arg)
{
  Server* self = static_cast<Server*>(arg);
  if (NULL == self)
    return 0;
  SetEvent(self->start_event_);

  while (self->running_) {
    FD_ZERO(&self->rset_);
    std::vector<SOCKET>::iterator it;
    for (it = self->fds_.begin(); it != self->fds_.end(); ) {
      if (INVALID_SOCKET == *it) {
        it = self->fds_.erase(it);
      }
      else {
        FD_SET(*it, &self->rset_);
        ++it;
      }
    }

    select(0, &self->rset_, NULL, NULL, NULL);

    int size = (int)self->fds_.size();
    for (int i = 0; i < size; ++i) {
      if (FD_ISSET(self->fds_[i], &self->rset_)) {
        if (!self->read_routine_(self->fds_[i], self)) {
          fprintf(stdout, "===> [%d] client quit\n", self->fds_[i]);
          shutdown(self->fds_[i], SD_BOTH);
          closesocket(self->fds_[i]);
          self->fds_[i] = INVALID_SOCKET;
        }
      }

      //! other events
    }
  }

  return 0;
}





static bool 
ReadRoutine(SOCKET s, void* arg)
{
  char buf[128] = {0};

  int ret = recv(s, buf, sizeof(buf), 0);
  if (SOCKET_ERROR == ret) 
    return false;

  fprintf(stdout, "recv from client : [%d] - %s\n", s, buf);
  send(s, buf, strlen(buf), 0);

  return true;
}

void 
ServerMain(const char* ip, unsigned short port)
{
  Server server;
  server.Start(ip, port);

  server.Run(ReadRoutine);

  server.Stop();
}
