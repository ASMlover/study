// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <boost/asio.hpp>
#include "msg_helper.hh"
#include "examples.hh"

namespace echo_server_asio_stackful {

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

awaitable<void> on_serve(tcp::socket socket) {
  try {
    co_await boost::asio::async_write(
        socket, boost::asio::buffer("*", 1), use_awaitable);

    auto status = coro::net::Status::WAIT_MSG;
    char rbuf[1024];
    for (;;) {
      auto n = co_await socket.async_read_some(
          boost::asio::buffer(rbuf), use_awaitable);

      auto [st, wbuf] = coro::msg::handle_message(status, rbuf, n);
      status = st;
      co_await boost::asio::async_write(
          socket, boost::asio::buffer(wbuf), use_awaitable);
    }
  }
  catch (const std::exception& /*e*/) {
    socket.close();
  }
}

awaitable<void> on_accept() {
  auto executor = co_await this_coro::executor;
  tcp::acceptor acceptor(executor, { tcp::v4(), 5555 });
  for (;;) {
    tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
    co_spawn(executor, [socket = std::move(socket)]() mutable {
          return on_serve(std::move(socket));
        }, detached);
  }
}

void launch() {
  try {
    boost::asio::io_context io_context;

    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&io_context](auto, auto) { io_context.stop(); });

    co_spawn(io_context, on_accept, detached);
    io_context.run();
  }
  catch (const std::exception& e) {
    std::cerr << "launch exception: " << e.what() << std::endl;
  }
}

}

CORO_EXAMPLE(EchoServerAsioStackful,
    esasiof, "an easy echo server use boost.asio with stackful coroutine") {
  echo_server_asio_stackful::launch();
}
