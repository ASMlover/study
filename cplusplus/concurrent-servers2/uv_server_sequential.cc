#include <uv.h>
#include "sequential_helper.hh"
#include "examples.hh"

namespace uss1 {

class Conn final : private coext::UnCopyable {
  uv_tcp_t* client_{};
  sequential::Status status_{ sequential::Status::INIT_ACK };
  std::vector<char> send_buf_;

  static void on_alloc_buffer(
    uv_handle_t* handle, coext::sz_t suggested_size, uv_buf_t* buf) {
    buf->base = (char*)malloc(suggested_size);
    buf->len = (ULONG)suggested_size;
  }
public:
  Conn(uv_tcp_t* client) noexcept : client_(client) {}

  inline uv_tcp_t* get_client() const { return client_; }

  bool on_connected(uv_write_cb cb) {
    status_ = sequential::Status::INIT_ACK;
    send_buf_.clear();
    send_buf_.push_back('*');

    uv_buf_t write_buf = uv_buf_init(send_buf_.data(), (unsigned int)send_buf_.size());
    uv_write_t* write_req = (uv_write_t*)malloc(sizeof(*write_req));
    write_req->data = this;
    if (uv_write(write_req, (uv_stream_t*)client_, &write_buf, 1, cb) < 0)
      return false;
    return true;
  }

  bool on_init_read(uv_read_cb cb) {
    status_ = sequential::Status::WAIT_MSG;
    send_buf_.clear();

    if (uv_read_start((uv_stream_t*)client_, on_alloc_buffer, cb) < 0)
      return false;
    return true;
  }

  bool on_read(const char* buf, int len, uv_write_cb cb) {
    if (status_ == sequential::Status::INIT_ACK)
      return true;

    for (int i = 0; i < len; ++i) {
      switch (status_) {
      case sequential::Status::INIT_ACK: break;
      case sequential::Status::WAIT_MSG:
        if (buf[i] == '^')
          status_ = sequential::Status::READ_MSG;
        break;
      case sequential::Status::READ_MSG:
        if (buf[i] == '$')
          status_ = sequential::Status::WAIT_MSG;
        else
          send_buf_.push_back(buf[i] + 1);
        break;
      }
    }

    if (!send_buf_.empty()) {
      uv_buf_t write_buf = uv_buf_init(send_buf_.data(), (unsigned int)send_buf_.size());
      uv_write_t* write_req = (uv_write_t*)malloc(sizeof(*write_req));
      write_req->data = this;
      if (uv_write(write_req, (uv_stream_t*)client_, &write_buf, 1, cb) < 0)
        return false;
    }
    return true;
  }

  void on_write() {
    send_buf_.clear();
  }
};

void on_client_closed(uv_handle_t* handle) {
  uv_tcp_t* client = (uv_tcp_t*)handle;
  if (client->data)
    delete (Conn*)client->data;
  free(client);
}

void on_client_write(uv_write_t* write_req, int status) {
  if (status < 0) {
    std::cerr << "write message to peer failed: " << uv_strerror(status) << std::endl;
    return;
  }

  std::unique_ptr<uv_write_t, std::function<void(uv_write_t*)>> wreq{ write_req, [](uv_write_t* w) { free(w); } };
  Conn* conn = (Conn*)wreq->data;
  conn->on_write();
}

void on_client_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
  std::unique_ptr<char, std::function<void(char*)>> bufp{ buf->base, [](char* p) { free(p); } };

  if (nread < 0) {
    uv_close((uv_handle_t*)client, on_client_closed);
  }
  else if (nread == 0) {
  }
  else {
    Conn* conn = (Conn*)client->data;
    if (!conn->on_read(buf->base, (int)nread, on_client_write))
      uv_close((uv_handle_t*)client, on_client_closed);
  }
}

void on_client_initack(uv_write_t* write_req, int status) {
  if (status < 0) {
    std::cerr << "write init ack error: " << uv_strerror(status) << std::endl;
    return;
  }

  std::unique_ptr<uv_write_t, std::function<void (uv_write_t*)>> wreq{
    write_req, [](uv_write_t* req) { free(req); }
  };

  Conn* conn = (Conn*)wreq->data;
  if (!conn->on_init_read(on_client_read))
    uv_close((uv_handle_t*)conn->get_client(), on_client_closed);
}

void on_client_connected(uv_stream_t* server, int status) {
  if (status < 0) {
    std::cerr << "peer connection error: " << uv_strerror(status) << std::endl;
    return;
  }

  uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(*client));
  if (uv_tcp_init(uv_default_loop(), client) < 0)
    return;
  client->data = nullptr;

  if (uv_accept(server, (uv_stream_t*)client) == 0) {
    Conn* conn = new Conn{ client };
    client->data = conn;
    if (!conn->on_connected(on_client_initack))
      uv_close((uv_handle_t*)client, on_client_closed);
  }
  else {
    uv_close((uv_handle_t*)client, on_client_closed);
  }
}

void launch_server() {
  uv_tcp_t server_handle;
  if (uv_tcp_init(uv_default_loop(), &server_handle) < 0)
    return;

  sockaddr_in addr;
  if (uv_ip4_addr("0.0.0.0", 5555, &addr) < 0)
    return;
  if (uv_tcp_bind(&server_handle, (const sockaddr*)&addr, sizeof(addr)) < 0)
    return;

  if (uv_listen((uv_stream_t*)&server_handle, 5, on_client_connected) < 0)
    return;

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  uv_loop_close(uv_default_loop());
}

}

COEXT_EXAMPLE(UVSequentialServer, uss1, "A sequential server with libuv") {
  uss1::launch_server();
}