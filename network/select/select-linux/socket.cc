#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "socket.h"




Socket::Socket(void)
  : socket_(0)
{
}

Socket::~Socket(void)
{
  Close();
}

bool 
Socket::Create(void)
{
  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  return (-1 != socket_);
}

void 
Socket::Close(void)
{
  shutdown(socket_, SHUT_RDWR);
  close(socket_);
}

bool 
Socket::Listen(const char* ip, unsigned int port)
{
  struct sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port);
  if (0 != bind(socket_, (struct sockaddr*)&addr, sizeof(addr)))
    return false;
  if (0 != listen(socket_, 20))
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
  if (0 != connect(socket_, (struct sockaddr*)&addr, sizeof(addr)))
    return false;
  return true;
}

int 
Socket::Accept(void)
{
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);

  return accept(socket_, (struct sockaddr*)&addr, &len);
}

int 
Socket::Read(int len, char* buf)
{
  return recv(socket_, buf, len, 0);
}

int 
Socket::Write(const char* buf, int len)
{
  return send(socket_, buf, len, 0);
}
