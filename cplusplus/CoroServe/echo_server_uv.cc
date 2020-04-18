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
#include <uv.h>
#include "msg_helper.hh"
#include "examples.hh"

namespace echo_server_uv {

static void on_client_closed(uv_handle_t* h);

class Conn final : private coro::UnCopyable {
  uv_tcp_t* client_{};
  coro::net::Status status_{coro::net::Status::INIT_ACK};
  coro::msg::WriteBuf wbuf_;

  template <typename T> class UVPtr final : private coro::UnCopyable {
    T* uv_obj_{};
  public:
    UVPtr(T* obj) noexcept : uv_obj_(obj) {}
    ~UVPtr() noexcept { free(uv_obj_); }
  };

  void clear_wbuf() { wbuf_.clear(); }

  static void on_alloc_buffer(
    uv_handle_t*, coro::sz_t suggested_size, uv_buf_t* buf) {
    buf->base = (char*)malloc(suggested_size);
#if defined(CORO_WIN)
    buf->len = (ULONG)suggested_size;
#else
    buf->len = suggested_size;
#endif
  }

  static void on_write_initack(uv_write_t* req, int err) {
    if (err < 0) {
      std::cerr << "write init ack error: " << uv_strerror(err) << std::endl;
      return;
    }

    UVPtr<uv_write_t> wreq{ req };
    Conn* conn = (Conn*)req->data;
    if (!conn->on_start_read())
      uv_close((uv_handle_t*)conn->get_client(), on_client_closed);
  }

  static void on_client_read(
    uv_stream_t* client, coro::ssz_t nread, const uv_buf_t* buf) {
    UVPtr<char> raw_buf{ buf->base };

    if (nread < 0) {
      uv_close((uv_handle_t*)client, on_client_closed);
    }
    else if (nread == 0) {
    }
    else {
      Conn* conn = (Conn*)client->data;
      if (!conn->on_read(buf->base, buf->len))
        uv_close((uv_handle_t*)client, on_client_closed);
    }
  }

  static void on_client_write(uv_write_t* req, int err) {
    if (err < 0) {
      std::cerr << "write message error: " << uv_strerror(err) << std::endl;
      return;
    }

    UVPtr<uv_write_t> wreq{ req };
    ((Conn*)req->data)->clear_wbuf();
  }
public:
  Conn(uv_tcp_t* c) noexcept : client_(c) {}
  inline uv_tcp_t* get_client() const noexcept { return client_; }

  bool do_write(uv_write_cb cb) {
    if (!wbuf_.empty()) {
      uv_buf_t wbuf = uv_buf_init(wbuf_.data(), (unsigned int)wbuf_.size());
      uv_write_t* req = (uv_write_t*)malloc(sizeof(*req));
      req->data = this;
      if (uv_write(req, (uv_stream_t*)client_, &wbuf, 1, cb) < 0)
        return false;
    }
    return true;
  }

  bool on_connected() {
    status_ = coro::net::Status::INIT_ACK;
    wbuf_.clear();
    wbuf_.push_back('*');

    return do_write(&Conn::on_write_initack);
  }

  bool on_start_read() {
    status_ = coro::net::Status::WAIT_MSG;
    wbuf_.clear();

    if (uv_read_start((uv_stream_t*)client_,
      &Conn::on_alloc_buffer, &Conn::on_client_read) < 0)
      return false;
    return true;
  }

  bool on_read(const char* rbuf, int rlen) {
    if (status_ == coro::net::Status::INIT_ACK)
      return true;

    auto [st, buf] = coro::msg::handle_message(status_, rbuf, rlen);
    status_ = st;
    coro::msg::buf::append(wbuf_, buf);

    return do_write(&Conn::on_client_write);
  }
};

static void on_client_closed(uv_handle_t* h) {
  uv_tcp_t* c = (uv_tcp_t*)h;
  if (c->data)
    delete (Conn*)c->data;
  free(c);
}

class EventLoop final : private coro::UnCopyable {
  uv_tcp_t server_{};

  static void on_accept(uv_stream_t* server, int err) {
    if (err < 0) {
      std::cerr << "peer connection error: " << uv_strerror(err) << std::endl;
      return;
    }

    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(*client));
    if (uv_tcp_init(uv_default_loop(), client) < 0)
      return;
    client->data = nullptr;

    if (uv_accept(server, (uv_stream_t*)client) == 0) {
      Conn* conn = new Conn{client};
      client->data = conn;
      if (!conn->on_connected())
        uv_close((uv_handle_t*)client, on_client_closed);
    }
    else {
      uv_close((uv_handle_t*)client, on_client_closed);
    }
  }

  void close() {
    auto close_cb = [](uv_handle_t*) {};
    uv_close((uv_handle_t*)&server_, close_cb);
  }
public:
  EventLoop() { uv_tcp_init(uv_default_loop(), &server_); }
  ~EventLoop() { close(); }

  bool create_server(
    coro::strv_t host = "0.0.0.0", int port = 5555, int backlog = 5) {
    sockaddr_in addr;
    if (uv_ip4_addr(host.data(), port, &addr) < 0)
      return false;
    if (uv_tcp_bind(&server_, (const sockaddr*)&addr, sizeof(addr)) < 0)
      return false;
    if (uv_listen((uv_stream_t*)&server_, backlog, &EventLoop::on_accept) < 0)
      return false;
    return true;
  }

  void run() {
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    uv_loop_close(uv_default_loop());
  }
};

void launch() {
  EventLoop loop{};

  loop.create_server();
  loop.run();
}

}

CORO_EXAMPLE(EchoServerUV, esuv, "an easy echo server use libuv") {
  echo_server_uv::launch();
}
