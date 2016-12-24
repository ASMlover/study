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
#include <assert.h>
#include <stdio.h>
#include <functional>
#include "../basic/TLogging.h"
#include "TSocketSupport.h"
#include "TConnector.h"
#include "TEventLoop.h"
#include "TInetAddress.h"
#include "TTcpClient.h"

namespace tyr { namespace net {

static void s_remove_connection(EventLoop* loop, const TcpConnectionPtr& conn) {
  loop->put_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
}

static void s_remove_connector(const ConnectorPtr& connector) {
}

TcpClient::TcpClient(EventLoop* loop, const InetAddress& server_addr)
  : loop_(TCHECK_NOTNULL(loop))
  , connector_(new Connector(loop, server_addr)) {
  connector_->set_new_connection_callback(std::bind(&TcpClient::new_connection, this, std::placeholders::_1));
  TYRLOG_INFO << "TcpClient::TcpClient [" << this << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient(void) {
  TYRLOG_INFO << "TcpClient::~TcpClient [" << this << "] - connector " << get_pointer(connector_);

  TcpConnectionPtr conn;
  {
    basic::MutexGuard guard(mtx_);
    conn = connection_;
  }

  if (conn) {
    CloseCallback close_fn = std::bind(&s_remove_connection, loop_, std::placeholders::_1);
    loop_->run_in_loop(std::bind(&TcpConnection::set_close_callback, conn, close_fn));
  }
  else {
    connector_->stop();
    loop_->run_after(1, std::bind(&s_remove_connector, connector_));
  }
}

void TcpClient::connect(void) {
  TYRLOG_INFO << "TcpClient::connect [" << this << "] - connecting to "
    << connector_->get_server_address().to_host_port();

  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect(void) {
  connect_ = false;

  {
    basic::MutexGuard guard(mtx_);
    if (connection_)
      connection_->shutdown();
  }
}

void TcpClient::stop(void) {
  connect_ = false;
  connector_->stop();
}

void TcpClient::new_connection(int sockfd) {
  loop_->assert_in_loopthread();
  InetAddress peer_addr(SocketSupport::kern_peekaddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof(buf), ":%s#%d", peer_addr.to_host_port().c_str(), next_connid_);
  ++next_connid_;
  std::string conn_name(buf);

  InetAddress local_addr(SocketSupport::kern_localaddr(sockfd));
  TcpConnectionPtr conn(new TcpConnection(loop_, conn_name, sockfd, local_addr, peer_addr));
  conn->set_connection_callback(connection_fn_);
  conn->set_message_callback(message_fn_);
  conn->set_write_complete_callback(write_complete_fn_);
  conn->set_close_callback(std::bind(&TcpClient::remove_connection, this, std::placeholders::_1));

  {
    basic::MutexGuard guard(mtx_);
    connection_ = conn;
  }
  conn->connect_established();
}

void TcpClient::remove_connection(const TcpConnectionPtr& conn) {
  loop_->assert_in_loopthread();
  assert(loop_ == conn->get_loop());

  {
    basic::MutexGuard guard(mtx_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->put_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
  if (retry_ && connect_) {
    TYRLOG_INFO << "TcpClient::remove_connection [" << this << "] - reconnecting to "
      << connector_->get_server_address().to_host_port();
    connector_->restart();
  }
}

}}
