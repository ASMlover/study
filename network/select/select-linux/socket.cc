#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "socket.h"




Socket::Socket(void)
  : fd_(0)
{
}

Socket::~Socket(void)
{
  Close();
}

bool 
Socket::Create(void)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  return (-1 != fd_);
}

void 
Socket::Close(void)
{
  shutdown(fd_, SHUT_RDWR);
  close(fd_);
}

bool 
Socket::Listen(const char* ip, unsigned int port)
{
  struct sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port);
  if (0 != bind(fd_, (struct sockaddr*)&addr, sizeof(addr)))
    return false;
  if (0 != listen(fd_, SOMAXCONN))
    return false;

  return true;
}

bool 
Socket::Connect(const char* ip, unsigned int port)
{
  struct sockaddr_in addr;

  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port);
  if (0 != connect(fd_, (struct sockaddr*)&addr, sizeof(addr)))
    return false;
  return true;
}

int 
Socket::Accept(void)
{
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);

  return accept(fd_, (struct sockaddr*)&addr, &len);
}

int 
Socket::Read(int len, char* buf)
{
  return recv(fd_, buf, len, 0);
}

int 
Socket::Write(const char* buf, int len)
{
  return send(fd_, buf, len, 0);
}
