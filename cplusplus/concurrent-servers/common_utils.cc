#include "common_utils.hh"

#pragma comment(lib, "ws2_32.lib")

namespace common {

WSGuarder::WSGuarder() noexcept {
  WSADATA wd;
  (void)WSAStartup(MAKEWORD(2, 2), &wd);
}

WSGuarder::~WSGuarder() noexcept {
  (void)WSACleanup();
}

UniqueSocket::~UniqueSocket() noexcept {
  if (sockfd_ != INVALID_SOCKET) {
    shutdown(sockfd_, SD_BOTH);
    closesocket(sockfd_);
  }
}

}