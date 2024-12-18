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
#include "netpp_internal.h"
#include "buffer.h"
#include "primitive.h"
#include "address.h"
#include "socket.h"
#include "socket_service.h"
#include <iostream>

namespace netpp {

struct AcceptOperation : public BaseOperation {
  BaseSocket& peer;
  Address& peer_addr;
  AcceptHandler handler{};

  AcceptOperation(int e, BaseSocket& s, Address& addr, const AcceptHandler& h)
    : BaseOperation(OPER_ACCEPT, e), peer(s), peer_addr(addr), handler(h) {
  }

  AcceptOperation(int e, BaseSocket& s, Address& addr, AcceptHandler&& h)
    : BaseOperation(OPER_ACCEPT, e)
    , peer(s), peer_addr(addr), handler(std::move(h)) {
  }
};

struct ConnectOperation : public BaseOperation {
  ConnectHandler handler{};

  ConnectOperation(int e, const ConnectHandler& h)
    : BaseOperation(OPER_CONNECT, e), handler(h) {
  }

  ConnectOperation(int e, ConnectHandler&& h)
    : BaseOperation(OPER_CONNECT, e), handler(std::move(h)) {
  }
};

struct ReadOperation : public BaseOperation {
  const MutableBuffer& mbuf;
  std::size_t nread{};
  ReadHandler handler{};

  ReadOperation(int e, const MutableBuffer& buf, const ReadHandler& h)
    : BaseOperation(OPER_READ, e), mbuf(buf), handler(h) {
  }

  ReadOperation(int e, const MutableBuffer& buf, ReadHandler&& h)
    : BaseOperation(OPER_READ, e), mbuf(buf), handler(std::move(h)) {
  }
};

struct ReadSomeOperation : public BaseOperation {
  const MutableBuffer& mbuf;
  ReadHandler handler{};

  ReadSomeOperation(int e, const MutableBuffer& buf, const ReadHandler& h)
    : BaseOperation(OPER_READ_SOME, e), mbuf(buf), handler(h) {
  }

  ReadSomeOperation(int e, const MutableBuffer& buf, ReadHandler&& h)
    : BaseOperation(OPER_READ_SOME, e), mbuf(buf), handler(std::move(h)) {
  }
};

struct WriteOperation : public BaseOperation {
  const ConstBuffer& cbuf;
  std::size_t nwrote{};
  WriteHandler handler{};

  WriteOperation(int e, const ConstBuffer& buf, const WriteHandler& h)
    : BaseOperation(OPER_WRITE, e), cbuf(buf), handler(h) {
  }

  WriteOperation(int e, const ConstBuffer& buf, WriteHandler&& h)
    : BaseOperation(OPER_WRITE, e), cbuf(buf), handler(std::move(h)) {
  }
};

struct WriteSomeOperation : public BaseOperation {
  const ConstBuffer& cbuf;
  WriteHandler handler{};

  WriteSomeOperation(int e, const ConstBuffer& buf, const WriteHandler& h)
    : BaseOperation(OPER_WRITE_SOME, e), cbuf(buf), handler(h) {
  }

  WriteSomeOperation(int e, const ConstBuffer& buf, WriteHandler&& h)
    : BaseOperation(OPER_WRITE_SOME, e), cbuf(buf), handler(std::move(h)) {
  }
};

struct ReadFromOperation : public BaseOperation {
  const MutableBuffer& mbuf;
  const Address& peer_addr;
  ReadHandler handler{};

  ReadFromOperation(int e,
      const MutableBuffer& buf, const Address& addr, const ReadHandler& h)
    : BaseOperation(OPER_READFROM, e), mbuf(buf), peer_addr(addr), handler(h) {
  }

  ReadFromOperation(int e,
      const MutableBuffer& buf, const Address& addr, ReadHandler&& h)
    : BaseOperation(OPER_READFROM, e)
    , mbuf(buf), peer_addr(addr), handler(std::move(h)) {
  }
};

struct WriteToOperation : public BaseOperation {
  const ConstBuffer& cbuf;
  const Address& peer_addr;
  WriteHandler handler{};

  WriteToOperation(int e,
      const ConstBuffer& buf, const Address& addr, const WriteHandler& h)
    : BaseOperation(OPER_WRITETO, e), cbuf(buf), peer_addr(addr), handler(h) {
  }

  WriteToOperation(int e,
      const ConstBuffer& buf, const Address& addr, WriteHandler&& h)
    : BaseOperation(OPER_WRITETO, e)
    , cbuf(buf), peer_addr(addr), handler(std::move(h)) {
  }
};

bool SocketService::add_operation(socket_t sockfd, BaseOperation* oper) {
  if (operations_.find(sockfd) == operations_.end())
    operations_[sockfd] = OperationDict();

  auto event = oper->get_events();
  if (operations_[sockfd].find(event) == operations_[sockfd].end()) {
    pollfds_.push_back(PollFd(sockfd, event));
    operations_[sockfd][event] = oper;
    return true;
  }

  return false;
}

bool SocketService::set_non_blocking(
    socket_t sockfd, bool mode, std::error_code& ec) {
  return socket::set_non_blocking(sockfd, mode, ec);
}

int SocketService::set_option(socket_t sockfd, int level, int optname,
      const void* optval, std::size_t optlen, std::error_code& ec) {
  return socket::set_option(sockfd, level, optname, optval, optlen, ec);
}

int SocketService::get_option(socket_t sockfd, int level, int optname,
      void* optval, std::size_t* optlen, std::error_code& ec) {
  return socket::get_option(sockfd, level, optname, optval, optlen, ec);
}

socket_t SocketService::open(
    int family, int socket_type, int protocol, std::error_code& ec) {
  return socket::open(family, socket_type, protocol, ec);
}

void SocketService::close(socket_t sockfd, std::error_code& ec) {
  socket::close(sockfd, ec);
}

void SocketService::shutdown(socket_t sockfd, int how, std::error_code& ec) {
  socket::shutdown(sockfd, how, ec);
}

void SocketService::bind(
    socket_t sockfd, const Address& addr, std::error_code& ec) {
  socket::bind(sockfd, addr.get_address(), ec);
}

void SocketService::listen(socket_t sockfd, std::error_code& ec) {
  socket::listen(sockfd, ec);
}

socket_t SocketService::accept(socket_t sockfd,
    Address* peer_addr, bool non_blocking, std::error_code& ec) {
  return socket::sync_accept(sockfd,
      peer_addr ? peer_addr->get_address() : nullptr, non_blocking, ec);
}

void SocketService::async_accept(socket_t sockfd,
    BaseSocket& peer, Address& peer_addr, const AcceptHandler& handler) {
  // sockfd should be non-blocking
  auto* oper = new AcceptOperation(POLLIN, peer, peer_addr, handler);
  add_operation(sockfd, oper);
}

void SocketService::async_accept(socket_t sockfd,
    BaseSocket& peer, Address& peer_addr, AcceptHandler&& handler) {
  // sockfd should be non-blocking
  auto* oper = new AcceptOperation(POLLIN, peer, peer_addr, std::move(handler));
  add_operation(sockfd, oper);
}

void SocketService::connect(
    socket_t sockfd, const Address& addr, std::error_code& ec) {
  socket::sync_connect(sockfd, addr.get_address(), ec);
}

void SocketService::async_connect(
    socket_t sockfd, const Address& addr, const ConnectHandler& handler) {
  // sockfd should be non-blocking
  std::error_code ec;
  socket::connect(sockfd, addr.get_address(), ec);
  if (!ec) {
    handler(ec);
  }
  else {
    if (ec.value() != error::IN_PROGRESS && ec.value() != error::WOULD_BLOCK) {
      handler(ec);
    }
    else {
      auto* oper = new ConnectOperation(POLLIN, handler);
      add_operation(sockfd, oper);
    }
  }
}

void SocketService::async_connect(
    socket_t sockfd, const Address& addr, ConnectHandler&& handler) {
  // sockfd should be non-blocking
  std::error_code ec;
  socket::connect(sockfd, addr.get_address(), ec);
  if (!ec) {
    handler(ec);
  }
  else {
    if (ec.value() != error::IN_PROGRESS && ec.value() != error::WOULD_BLOCK) {
      handler(ec);
    }
    else {
      auto* oper = new ConnectOperation(POLLIN, std::move(handler));
      add_operation(sockfd, oper);
    }
  }
}

std::size_t SocketService::read(socket_t sockfd,
    const MutableBuffer& buf, bool non_blocking, std::error_code& ec) {
  auto all_empty = buf.size() == 0;
  std::size_t nread{};
  for (;;) {
    auto n = socket::sync_read(sockfd,
        buf.size() - nread, reinterpret_cast<char*>(buf.data()) + nread,
        non_blocking, all_empty, ec);
    if (!ec) {
      nread += n;
      if (nread >= buf.size())
        break;
    }
    else {
      break;
    }
  }
  return nread;
}

std::size_t SocketService::read(socket_t sockfd,
    const NullBuffer&, bool non_blocking, std::error_code& ec) {
  return socket::poll_read(sockfd, non_blocking, -1, ec);
}

std::size_t SocketService::read_some(socket_t sockfd,
    const MutableBuffer& buf, bool non_blocking, std::error_code& ec) {
  auto all_empty = buf.size() == 0;
  return socket::sync_read(sockfd,
      buf.size(), buf.data(), non_blocking, all_empty, ec);
}

std::size_t SocketService::read_some(socket_t sockfd,
    const NullBuffer&, bool non_blocking, std::error_code& ec) {
  return socket::poll_read(sockfd, non_blocking, -1, ec);
}

void SocketService::async_read(
    socket_t sockfd, const MutableBuffer& buf, const ReadHandler& handler) {
  // sockfd should be non-blocking
  auto* oper = new ReadOperation(POLLIN, buf, handler);
  add_operation(sockfd, oper);
}

void SocketService::async_read(
    socket_t sockfd, const MutableBuffer& buf, ReadHandler&& handler) {
  // sockfd should be non-blocking
  auto* oper = new ReadOperation(POLLIN, buf, std::move(handler));
  add_operation(sockfd, oper);
}

void SocketService::async_read_some(
    socket_t sockfd, const MutableBuffer& buf, const ReadHandler& handler) {
  // sockfd should be non-blocking
  auto* oper = new ReadSomeOperation(POLLIN, buf, handler);
  add_operation(sockfd, oper);
}

void SocketService::async_read_some(
    socket_t sockfd, const MutableBuffer& buf, ReadHandler&& handler) {
  // sockfd should be non-blocking
  auto* oper = new ReadSomeOperation(POLLIN, buf, std::move(handler));
  add_operation(sockfd, oper);
}

std::size_t SocketService::write(socket_t sockfd,
    const ConstBuffer& buf, bool non_blocking, std::error_code& ec) {
  auto all_empty = buf.size() == 0;
  std::size_t nwrote{};
  for (;;) {
    auto n = socket::sync_write(sockfd,
        reinterpret_cast<const char*>(buf.data()) + nwrote,
        buf.size() - nwrote, non_blocking, all_empty, ec);
    if (!ec) {
      nwrote += n;
      if (nwrote >= buf.size())
        break;
    }
    else {
      break;
    }
  }
  return nwrote;
}

std::size_t SocketService::write(socket_t sockfd,
      const NullBuffer&, bool non_blocking, std::error_code& ec) {
  return socket::poll_write(sockfd, non_blocking, -1, ec);
}

std::size_t SocketService::write_some(socket_t sockfd,
    const ConstBuffer& buf, bool non_blocking, std::error_code& ec) {
  auto all_empty = buf.size() == 0;
  return socket::sync_write(sockfd,
      buf.data(), buf.size(), non_blocking, all_empty, ec);
}

std::size_t SocketService::write_some(socket_t sockfd,
    const NullBuffer&, bool non_blocking, std::error_code& ec) {
  return socket::poll_write(sockfd, non_blocking, -1, ec);
}

void SocketService::async_write(
    socket_t sockfd, const ConstBuffer& buf, const WriteHandler& handler) {
  // sockfd should be non-blocking
  auto* oper = new WriteOperation(POLLOUT, buf, handler);
  add_operation(sockfd, oper);
}

void SocketService::async_write(
    socket_t sockfd, const ConstBuffer& buf, WriteHandler&& handler) {
  // sockfd should be non-blocking
  auto* oper = new WriteOperation(POLLOUT, buf, std::move(handler));
  add_operation(sockfd, oper);
}

void SocketService::async_write_some(
    socket_t sockfd, const ConstBuffer& buf, const WriteHandler& handler) {
  // sockfd should be non-blocking
  auto* oper = new WriteSomeOperation(POLLOUT, buf, handler);
  add_operation(sockfd, oper);
}

void SocketService::async_write_some(
    socket_t sockfd, const ConstBuffer& buf, WriteHandler&& handler) {
  // sockfd should be non-blocking
  auto* oper = new WriteSomeOperation(POLLOUT, buf, std::move(handler));
  add_operation(sockfd, oper);
}

std::size_t SocketService::read_from(socket_t sockfd, const MutableBuffer& buf,
    Address& peer_addr, bool non_blocking, std::error_code& ec) {
  return socket::sync_read_from(sockfd,
      buf.size(), buf.data(), peer_addr.get_address(), non_blocking, ec);
}

std::size_t SocketService::read_from(socket_t sockfd,
      const NullBuffer&, Address&, bool non_blocking, std::error_code& ec) {
  socket::poll_read(sockfd, non_blocking, -1, ec);
  return 0;
}

void SocketService::async_read_from(socket_t sockfd,
    const MutableBuffer& buf, Address& peer_addr, const ReadHandler& handler) {
  // sockfd should be non-blocking
  auto* oper = new ReadFromOperation(POLLIN, buf, peer_addr, handler);
  add_operation(sockfd, oper);
}

void SocketService::async_read_from(socket_t sockfd,
    const MutableBuffer& buf, Address& peer_addr, ReadHandler& handler) {
  // sockfd should be non-blocking
  auto* oper = new ReadFromOperation(
      POLLIN, buf, peer_addr, std::move(handler));
  add_operation(sockfd, oper);
}

std::size_t SocketService::write_to(socket_t sockfd, const ConstBuffer& buf,
    const Address& peer_addr, bool non_blocking, std::error_code& ec) {
  return socket::sync_write_to(sockfd,
      buf.data(), buf.size(), peer_addr.get_address(), non_blocking, ec);
}

std::size_t SocketService::write_to(socket_t sockfd,
    const NullBuffer&, const Address&, bool non_blocking, std::error_code& ec) {
  socket::poll_write(sockfd, non_blocking, -1, ec);
  return 0;
}

void SocketService::async_write_to(socket_t sockfd, const ConstBuffer& buf,
    const Address& peer_addr, const WriteHandler& handler) {
  // sockfd should be non-blocking
  auto* oper = new WriteToOperation(POLLOUT, buf, peer_addr, handler);
  add_operation(sockfd, oper);
}

void SocketService::async_write_to(socket_t sockfd,
    const ConstBuffer& buf, const Address& peer_addr, WriteHandler&& handler) {
  // sockfd should be non-blocking
  auto* oper = new WriteToOperation(
      POLLOUT, buf, peer_addr, std::move(handler));
  add_operation(sockfd, oper);
}

bool SocketService::handle_operation(socket_t sockfd, int event) {
  auto oper_dict_iter = operations_.find(sockfd);
  if (oper_dict_iter == operations_.end())
    return false;

  auto& oper_dict = operations_[sockfd];
  auto oper_iter = oper_dict.find(event);
  bool need_remove{true};
  if (oper_iter != oper_dict.end()) {
    auto* oper = oper_iter->second;
    if (oper != nullptr) {
      std::error_code ec;
      switch (oper->get_type()) {
      case BaseOperation::OPER_ACCEPT:
        {
          auto* aop = reinterpret_cast<AcceptOperation*>(oper);
          auto fd = socket::accept(sockfd, aop->peer_addr.get_address(), ec);
          aop->peer.attach_fd(fd);
          aop->handler(ec);
          delete aop;
        } break;
      case BaseOperation::OPER_CONNECT:
        {
          auto* cop = reinterpret_cast<ConnectOperation*>(oper);
          cop->handler(ec);
          delete cop;
        } break;
      case BaseOperation::OPER_READ:
        {
          auto* rop = reinterpret_cast<ReadOperation*>(oper);
          auto off = rop->nread;
          auto n = socket::read(sockfd, rop->mbuf.size() - off,
              reinterpret_cast<char*>(rop->mbuf.data()) + off, ec);
          if (!ec)
            rop->nread += n;
          if ((!ec && rop->nread >= rop->mbuf.size()) || ec) {
            rop->handler(ec, rop->nread);
            delete rop;
          }
          else {
            need_remove = false;
          }
        } break;
      case BaseOperation::OPER_READ_SOME:
        {
          auto* rop = reinterpret_cast<ReadSomeOperation*>(oper);
          auto n = socket::read(sockfd, rop->mbuf.size(), rop->mbuf.data(), ec);
          rop->handler(ec, n);
          delete rop;
        } break;
      case BaseOperation::OPER_WRITE:
        {
          auto* wop = reinterpret_cast<WriteOperation*>(oper);
          auto off = wop->nwrote;
          auto n = socket::write(sockfd,
              reinterpret_cast<const char*>(wop->cbuf.data()) + off,
              wop->cbuf.size() - off, ec);
          if (!ec)
            wop->nwrote += n;
          if ((!ec && wop->nwrote >= wop->cbuf.size()) || ec) {
            wop->handler(ec, wop->nwrote);
            delete wop;
          }
          else {
            need_remove = false;
          }
        } break;
      case BaseOperation::OPER_WRITE_SOME:
        {
          auto* wop = reinterpret_cast<WriteSomeOperation*>(oper);
          auto n = socket::write(sockfd,
              wop->cbuf.data(), wop->cbuf.size(), ec);
          wop->handler(ec, n);
          delete wop;
        } break;
      case BaseOperation::OPER_READFROM:
        {
          auto* rop = reinterpret_cast<ReadFromOperation*>(oper);
          auto n = socket::read_from(sockfd,
              rop->mbuf.size(), rop->mbuf.data(),
              (void*)rop->peer_addr.get_address(),
              ec, rop->peer_addr.get_family() == AF_INET6);
          rop->handler(ec, n);
          delete rop;
        } break;
      case BaseOperation::OPER_WRITETO:
        {
          auto* wop = reinterpret_cast<WriteToOperation*>(oper);
          auto n = socket::write_to(sockfd,
              wop->cbuf.data(), wop->cbuf.size(),
              wop->peer_addr.get_address(), ec);
          wop->handler(ec, n);
          delete wop;
        } break;
      }
    }
    if (need_remove)
      oper_dict.erase(oper_iter);
  }
  if (oper_dict.empty())
    operations_.erase(oper_dict_iter);

  return need_remove;
}

void SocketService::handle_error(socket_t sockfd) {
  auto oper_dict_iter = operations_.find(sockfd);
  if (oper_dict_iter == operations_.end())
    return;

  auto& oper_dict = operations_[sockfd];
  for (auto& it : oper_dict) {
    std::error_code ec = make_error(get_errno());
    auto* oper = it.second;
    switch (oper->get_type()) {
    case BaseOperation::OPER_ACCEPT:
      {
        auto* aop = reinterpret_cast<AcceptOperation*>(oper);
        aop->handler(ec);
        delete aop;
      } break;
    case BaseOperation::OPER_CONNECT:
      {
        auto* cop = reinterpret_cast<ConnectOperation*>(oper);
        cop->handler(ec);
        delete cop;
      } break;
    case BaseOperation::OPER_READ:
      {
        auto* rop = reinterpret_cast<ReadOperation*>(oper);
        rop->handler(ec, rop->nread);
        delete rop;
      } break;
    case BaseOperation::OPER_READ_SOME:
      {
        auto* rop = reinterpret_cast<ReadSomeOperation*>(oper);
        rop->handler(ec, 0);
        delete rop;
      } break;
    case BaseOperation::OPER_WRITE:
      {
        auto* wop = reinterpret_cast<WriteOperation*>(oper);
        wop->handler(ec, wop->nwrote);
        delete wop;
      } break;
    case BaseOperation::OPER_WRITE_SOME:
      {
        auto* wop = reinterpret_cast<WriteSomeOperation*>(oper);
        wop->handler(ec, 0);
        delete wop;
      } break;
    case BaseOperation::OPER_READFROM:
      {
        auto* rop = reinterpret_cast<ReadFromOperation*>(oper);
        rop->handler(ec, 0);
        delete rop;
      } break;
    case BaseOperation::OPER_WRITETO:
      {
        auto* wop = reinterpret_cast<WriteToOperation*>(oper);
        wop->handler(ec, 0);
        delete wop;
      } break;
    }
  }
  operations_.erase(oper_dict_iter);
}

void SocketService::run(void) {
  while (running_) {
    auto n = netpp::poll(&*pollfds_.begin(),
        static_cast<int>(pollfds_.size()), kPollTimeout);
    if (n <= 0)
      continue;

    std::vector<PollFd> active_fds;
    for (auto it = pollfds_.begin(); it != pollfds_.end();) {
      if (n-- <= 0)
        break;

      auto revents = it->revents;
      if (revents > 0) {
        active_fds.push_back(*it);
        pollfds_.erase(it++);
      }
      else {
        ++it;
      }
    }

    for (auto& fd : active_fds) {
        bool need_remove{true};
        auto revents = fd.revents;
        if ((revents & POLLHUP) && !(revents & POLLIN))  {
          // TODO: need solve close-functor
        }
        if (revents & (POLLERR | POLLNVAL)) // solve error-functor
          handle_error(fd.fd);
        if (revents & (POLLIN | POLLPRI | POLLHUP))
          need_remove = handle_operation(fd.fd, POLLIN);
        if (revents & POLLOUT)
          need_remove = handle_operation(fd.fd, POLLOUT);

        if (!need_remove)
          pollfds_.push_back(fd);
    }

    if (pollfds_.size() == 0 && operations_.size() == 0)
      running_ = false;
  }
}

void SocketService::stop(void) {
  running_ = false;
}

}
