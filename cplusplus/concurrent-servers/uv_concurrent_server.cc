#include <uv.h>
#include <vector>
#include "common_utils.hh"
#include "examples.hh"

namespace ucs {

enum class ConnState { INIT_ACK, WAIT_MSG, READ_MSG };

template <typename T>
class UVHandleGuarder final : private common::UnCopyable {
  T* handle_{};
public:
  UVHandleGuarder(T* handle) noexcept : handle_(handle) {}
  ~UVHandleGuarder() noexcept { free(handle_); }
};

struct Conn {
  uv_tcp_t* client{};
  ConnState state{ConnState::INIT_ACK};
  std::vector<char> buff;

  Conn(uv_tcp_t* c = nullptr) noexcept : client(c) {}

  inline void switch_state(ConnState s) { state = s; }
  inline void append_buff(char c) { buff.push_back(c); }
  inline bool need_write() const { return !buff.empty(); }

  void init_for_ack() {
    state = ConnState::INIT_ACK;
    buff.push_back('*');
  }

  void init_for_wait() {
    state = ConnState::WAIT_MSG;
    buff.clear();
  }

  void on_write() {
    buff.clear();
  }
};

void on_alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = (ULONG)suggested_size;
}

void on_client_closed(uv_handle_t* handle) {
  uv_tcp_t* client = (uv_tcp_t*)handle;
  if (client->data) {
    auto* conn = (Conn*)client->data;
    delete conn;
  }
  free(client);
}

void on_client_write(uv_write_t* req, int status) {
  UVHandleGuarder<uv_write_t> wguard(req);

  Conn* conn = (Conn*)req->data;
  conn->on_write();
}

void on_client_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
  UVHandleGuarder<char> bufguard(buf->base);

  if (nread < 0) {
    uv_close((uv_handle_t*)client, on_client_closed);
  }
  else if (nread == 0) {
  }
  else {
    Conn* conn = (Conn*)client->data;
    if (conn->state == ConnState::INIT_ACK) {
      return;
    }

    for (int i = 0; i < nread; ++i) {
      switch (conn->state) {
      case ConnState::INIT_ACK: break;
      case ConnState::WAIT_MSG:
        if (buf->base[i] == '^')
          conn->switch_state(ConnState::READ_MSG);
        break;
      case ConnState::READ_MSG:
        if (buf->base[i] == '$')
          conn->switch_state(ConnState::WAIT_MSG);
        else
          conn->append_buff(buf->base[i] + 1);
        break;
      }
    }

    if (conn->need_write()) {
      uv_buf_t write_buf = uv_buf_init(conn->buff.data(), (unsigned int)conn->buff.size());
      uv_write_t* wreq = (uv_write_t*)malloc(sizeof(*wreq));
      wreq->data = conn;
      if (auto rc = uv_write(wreq, client, &write_buf, 1, on_client_write); rc < 0)
        return;
    }
  }
}

void on_write_init_ack(uv_write_t* req, int status) {
  UVHandleGuarder<uv_write_t> wguard(req);

  Conn* conn = (Conn*)req->data;
  conn->init_for_wait();

  if (auto rc = uv_read_start((uv_stream_t*)conn->client, on_alloc_buffer, on_client_read); rc < 0)
    return;
}

void on_new_connection(uv_stream_t* server_stream, int status) {
  if (status < 0) {
    std::cerr << "peer connection error: " << uv_strerror(status) << std::endl;
    return;
  }

  uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(*client));
  if (auto rc = uv_tcp_init(uv_default_loop(), client); rc < 0)
    return;
  client->data = nullptr;

  if (uv_accept(server_stream, (uv_stream_t*)client) == 0) {
    Conn* conn = new Conn(client);
    conn->init_for_ack();
    client->data = conn;

    uv_buf_t write_buf = uv_buf_init(conn->buff.data(), (unsigned int)conn->buff.size());
    uv_write_t* req = (uv_write_t*)malloc(sizeof(*req));
    req->data = conn;
    if (auto rc = uv_write(req, (uv_stream_t*)client, &write_buf, 1, on_write_init_ack); rc < 0)
      return;
  }
  else {
    uv_close((uv_handle_t*)client, on_client_closed);
  }
}

void launch_server() {
  uv_tcp_t server_handle;
  if (auto rc = uv_tcp_init(uv_default_loop(), &server_handle); rc < 0)
    return;

  sockaddr_in server_addr;
  if (auto rc = uv_ip4_addr("0.0.0.0", 5555, &server_addr); rc < 0)
    return;
  if (auto rc = uv_tcp_bind(&server_handle, (const sockaddr*)&server_addr, 0); rc < 0)
    return;

  if (auto rc = uv_listen((uv_stream_t*)&server_handle, 5, on_new_connection); rc < 0)
    return;

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  uv_loop_close(uv_default_loop());
}

}

EFW_EXAMPLE(UVConcurrentServer, ucs) {
  ucs::launch_server();
}