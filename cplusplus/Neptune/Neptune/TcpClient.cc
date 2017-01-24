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
#include <functional>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/Connector.h>
#include <Neptune/EventLoop.h>
#include <Neptune/InetAddress.h>
#include <Neptune/TcpConnection.h>
#include <Neptune/TcpClient.h>

namespace Neptune {

static void invoke_remove_connection(EventLoop* loop, const TcpConnectionPtr& conn) {
  loop->put_in_loop(std::bind(&TcpConnection::do_connect_destroyed, conn));
}

static void invoke_remove_connector(const ConnectorPtr& /*connector*/) {
}


TcpClient::TcpClient(EventLoop* loop, const InetAddress& server_addr, const std::string& name)
  : loop_(CHAOS_CHECK_NONIL(loop))
  , connector_(new Connector(loop_, server_addr))
  , name_(name)
  , connection_fn_(on_connection_default)
  , message_fn_(on_message_default) {
  connector_->bind_new_connection_functor(
      std::bind(&TcpClient::do_handle_new_connection, this, std::placeholders::_1));
  CHAOSLOG_INFO << "TcpClient::TcpClient - [" << this << "] connector " << Chaos::get_pointer(connector_);
}

TcpClient::~TcpClient(void) {
  CHAOSLOG_INFO << "TcpClient::~TcpClient - [" << this << "] connector " << Chaos::get_pointer(connector_);

  TcpConnectionPtr conn;
  bool is_unique = false;
  {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    is_unique = connection_.unique();
    conn = connection_;
  }

  if (conn) {
    CHAOS_CHECK(loop_ == conn->get_loop(), "connection should in current event loop");
    loop_->run_in_loop([this, &conn](void) {
          conn->bind_close_functor(std::bind(&invoke_remove_connection, loop_, std::placeholders::_1));
        });

    if (is_unique)
      conn->force_close();
  }
  else {
    connector_->stop();
    loop_->run_after(1, std::bind(&invoke_remove_connector, connector_));
  }
}

void TcpClient::connect(void) {
  CHAOSLOG_INFO << "TcpClient::connect - [" << this << "] connecting to "
    << connector_->get_server_address().get_host_port();

  need_connect_ = true;
  connector_->start();
}

void TcpClient::disconnect(void) {
  need_connect_ = false;

  {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    if (connection_)
      connection_->shutdown();
  }
}

void TcpClient::stop(void) {
  need_connect_ = false;
  connector_->stop();
}

void TcpClient::do_handle_new_connection(int sockfd) {
  loop_->assert_in_loopthread();
  InetAddress peer_addr(NetOps::socket::get_peer(sockfd));
  char buf[32]{};
  std::snprintf(buf, sizeof(buf), "-%s#%d", peer_addr.get_host_port().c_str(), next_connid_);
  ++next_connid_;
  std::string conn_name = name_ + buf;

  InetAddress local_addr(NetOps::socket::get_local(sockfd));
  TcpConnectionPtr conn(new TcpConnection(loop_, conn_name, sockfd, local_addr, peer_addr));
  conn->bind_connection_functor(connection_fn_);
  conn->bind_message_functor(message_fn_);
  conn->bind_write_complete_functor(write_complete_fn_);
  conn->bind_close_functor(std::bind(&TcpClient::remove_connection, this, std::placeholders::_1));

  {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    connection_ = conn;
  }
  conn->do_connect_established();
}

void TcpClient::remove_connection(const TcpConnectionPtr& conn) {
  CHAOS_CHECK(loop_ == conn->get_loop(), "connection should in current event loop");
  loop_->assert_in_loopthread();

  {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    CHAOS_CHECK(connection_ == conn, "`connection_` should equal to `conn`");
    connection_.reset();
  }

  loop_->put_in_loop(std::bind(&TcpConnection::do_connect_destroyed, conn));
  if (retry_ && need_connect_) {
    CHAOSLOG_INFO << "TcpClient::remove_connection - [" << this << "] - reconnecting to "
      << connector_->get_server_address().get_host_port();
    connector_->restart();
  }
}

}
