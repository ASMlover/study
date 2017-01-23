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
#include <Neptune/Kern/NetOps.h>
#include <Neptune/InetAddress.h>
#include <Neptune/EventLoop.h>
#include <Neptune/Acceptor.h>

namespace Neptune {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port)
  : loop_(loop)
  , accept_socket_(NetOps::socket::open(listen_addr.get_family()))
  , accept_channel_(loop_, accept_socket_.get_fd()) {
  accept_socket_.set_reuse_addr(true);
  accept_socket_.set_reuse_port(reuse_port);
  accept_socket_.bind_address(listen_addr);
  accept_channel_.bind_read_functor(std::bind(&Acceptor::do_handle_read, this));
}

Acceptor::~Acceptor(void) {
  accept_channel_.disabled_all();
  accept_channel_.remove();
}

void Acceptor::listen(void) {
  loop_->assert_in_loopthread();

  listening_ = true;
  accept_socket_.listen();
  accept_channel_.enabled_reading();
}

void Acceptor::do_handle_read(void) {
  loop_->assert_in_loopthread();

  InetAddress peer_addr;
  int connfd = accept_socket_.accept(peer_addr);
  if (connfd >= 0){
    if (new_connection_fn_)
      new_connection_fn_(connfd, peer_addr);
    else
      NetOps::socket::close(connfd);
  }
}

}
