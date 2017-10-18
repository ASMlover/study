// Copyright (c) 2017 ASMlover. All rights reserved.
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
#pragma once

#include <memory>
#include <system_error>
#include <vector>
#include <map>
#include <unordered_map>
#include <Chaos/Base/UnCopyable.h>
#include "primitive.h"

namespace netpp {

class Address;
class BaseSocket;
class ConstBuffer;
class MutableBuffer;
class NullBuffer;

struct BaseOperation {
  enum {
    OPER_INVALID = -1,

    OPER_ACCEPT, OPER_CONNECT,
    OPER_READ, OPER_WRITE, OPER_READFROM, OPER_WRITETO,
  };

  int op_type{OPER_INVALID};
  int events{};

  BaseOperation(int ot, int e) : op_type(ot), events(e) {}

  int get_type(void) const { return op_type; }
  int get_events(void) const { return events; }
};

class SocketService : private Chaos::UnCopyable {
  using OperationDict = std::map<int, BaseOperation*>;

  bool running_{true};
  std::vector<PollFd> pollfds_;
  std::unordered_map<int, OperationDict> operations_;

  static constexpr int kPollTimeout = 10;

  bool add_operation(int sockfd, BaseOperation* oper);
  void handle_operation(int sockfd, int event);
public:
  bool set_non_blocking(socket_t sockfd, bool mode, std::error_code& ec);
  int set_option(socket_t sockfd, int level, int optname,
      const void* optval, std::size_t optlen, std::error_code& ec);
  int get_option(socket_t sockfd, int level, int optname,
      void* optval, std::size_t* optlen, std::error_code& ec);
  socket_t open(int family, int socket_type, int protocol, std::error_code& ec);
  void close(socket_t sockfd, std::error_code& ec);
  void shutdown(socket_t sockfd, int how, std::error_code& ec);
  void bind(socket_t sockfd, const Address& addr, std::error_code& ec);
  void listen(socket_t sockfd, std::error_code& ec);
  socket_t accept(socket_t sockfd,
      Address* peer_addr, bool non_blocking, std::error_code& ec);
  void async_accept(socket_t sockfd,
      BaseSocket& peer, Address& peer_addr, const AcceptHandler& handler);
  void async_accept(socket_t sockfd,
      BaseSocket& peer, Address& peer_addr, AcceptHandler&& handler);
  void connect(socket_t sockfd, const Address& addr, std::error_code& ec);
  void async_connect(
      socket_t sockfd, const Address& addr, const ConnectHandler& handler);
  void async_connect(
      socket_t sockfd, const Address& addr, ConnectHandler&& handler);
  std::size_t read(socket_t sockfd,
      const MutableBuffer& buf, bool non_blocking, std::error_code& ec);
  std::size_t read(socket_t sockfd,
      const NullBuffer&, bool non_blocking, std::error_code& ec);
  void async_read(
      socket_t sockfd, const MutableBuffer& buf, const ReadHandler& handler);
  void async_read(
      socket_t sockfd, const MutableBuffer& buf, ReadHandler&& handler);
  std::size_t write(socket_t sockfd,
      const ConstBuffer& buf, bool non_blocking, std::error_code& ec);
  std::size_t write(socket_t sockfd,
      const NullBuffer&, bool non_blocking, std::error_code& ec);
  void async_write(
      socket_t sockfd, const ConstBuffer& buf, const WriteHandler& handler);
  void async_write(
      socket_t sockfd, const ConstBuffer& buf, WriteHandler&& handler);
  std::size_t read_from(socket_t sockfd, const MutableBuffer& buf,
      Address& peer_addr, bool non_blocking, std::error_code& ec);
  std::size_t read_from(socket_t sockfd,
      const NullBuffer&, Address&, bool non_blocking, std::error_code& ec);
  void async_read_from(socket_t sockfd,
      const MutableBuffer& buf, Address& peer_addr, const ReadHandler& handler);
  void async_read_from(socket_t sockfd,
      const MutableBuffer& buf, Address& peer_addr, ReadHandler& handler);
  std::size_t write_to(socket_t sockfd, const ConstBuffer& buf,
      const Address& peer_addr, bool non_blocking, std::error_code& ec);
  std::size_t write_to(socket_t sockfd, const NullBuffer&,
      const Address&, bool non_blocking, std::error_code& ec);
  void async_write_to(socket_t sockfd, const ConstBuffer& buf,
      const Address& peer_addr, const WriteHandler& handler);
  void async_write_to(socket_t sockfd,
      const ConstBuffer& buf, const Address& peer_addr, WriteHandler&& handler);

  void run(void);
  void stop(void);
};

}
