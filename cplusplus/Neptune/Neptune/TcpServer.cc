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
#include <cstdio>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Acceptor.h>
#include <Neptune/EventLoop.h>
#include <Neptune/EventLoopThreadPool.h>
#include <Neptune/InetAddress.h>
#include <Neptune/TcpConnection.h>
#include <Neptune/TcpServer.h>

namespace Neptune {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& host_addr, const std::string& name, Option opt)
  : loop_(CHAOS_CHECK_NONIL(loop))
  , host_port_(host_addr.get_host_port())
  , name_(name)
  , acceptor_(new Acceptor(loop_, host_addr, opt == Option::OPTION_REUSEPORT))
  , loop_threadpool_(new EventLoopThreadPool(loop_, name_))
  , connection_fn_(on_connection_default)
  , message_fn_(on_message_default) {
  acceptor_->bind_new_connection_functor(
      std::bind(&TcpServer::do_handle_new_connection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer(void) {
  loop_->assert_in_loopthread();
  CHAOSLOG_TRACE << "TcpServer::~TcpServer - [" << name_ << "] destructing";

  for (auto& conn_pair : connections_) {
    TcpConnectionPtr conn = conn_pair.second;
    conn_pair.second.reset();
    conn->get_loop()->run_in_loop(std::bind(&TcpConnection::do_connect_destroyed, conn));
    conn.reset();
  }
}

void TcpServer::start(void) {
  if (started_++ == 0) {
    loop_threadpool_->start(initiate_fn_);

    CHAOS_CHECK(!acceptor_->is_listening(), "acceptor should not in listening");
    if (!acceptor_->is_listening())
      loop_->run_in_loop(std::bind(&Acceptor::listen, Chaos::get_pointer(acceptor_)));
  }
}

void TcpServer::set_nthreads(int nthreads) {
  CHAOS_CHECK(nthreads >= 0, "event loop thread should greater than 0");
  loop_threadpool_->set_nthreads(nthreads);
}

void TcpServer::do_handle_new_connection(int sockfd, const InetAddress& peer_addr) {
  loop_->assert_in_loopthread();

  char buf[64];
  std::snprintf(buf, sizeof(buf), "-%s#%d", host_port_.c_str(), next_connid_);
  ++next_connid_;
  std::string conn_name = name_ + buf;

  CHAOSLOG_INFO << "TcpServer::do_handle_new_connection - [" << name_
    << "] new connection [" << conn_name << "] from " << peer_addr.get_host_port();
  InetAddress local_addr(NetOps::socket::get_local(sockfd));

  EventLoop* loop = loop_threadpool_->get_next_loop();
  TcpConnectionPtr conn(new TcpConnection(loop, conn_name, sockfd, local_addr, peer_addr));
  connections_[conn_name] = conn;
  conn->bind_connection_functor(connection_fn_);
  conn->bind_message_functor(message_fn_);
  conn->bind_write_complete_functor(write_complete_fn_);
  conn->bind_close_functor(std::bind(&TcpServer::remove_connection, this, std::placeholders::_1));
  loop->run_in_loop(std::bind(&TcpConnection::do_connect_established, conn));
}

void TcpServer::remove_connection(const TcpConnectionPtr& conn) {
  loop_->run_in_loop([this, &conn](void) {
        loop_->assert_in_loopthread();
        CHAOSLOG_INFO << "TcpServer::remove_connection - [" << name_
          << "] connection " << conn->get_name();
        std::size_t n = connections_.erase(conn->get_name());
        CHAOS_UNUSED(n);
        EventLoop* loop = conn->get_loop();
        loop->put_in_loop(std::bind(&TcpConnection::do_connect_destroyed, conn));
      });
}

}
