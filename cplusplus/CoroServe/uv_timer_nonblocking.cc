#include <uv.h>
#include "examples.hh"

namespace uv_timer_nblock {

void on_work(uv_work_t* work_req) {
  srand((unsigned int)time(nullptr));
  if (rand() % 3 == 0) {
    std::cout << "uv_timer: begin sleeping ..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "uv_timer: end sleeping ..." << std::endl;
  }
}

void on_after_work(uv_work_t* work_req, int status) {
  free(work_req);
}

void on_timer(uv_timer_t* timer) {
  auto timestamp = uv_hrtime();
  std::cout << "uv_timer: " << (timestamp / 1000000) % 1000000 << " ms" << std::endl;

  uv_work_t* req = (uv_work_t*)malloc(sizeof(*req));
  uv_queue_work(uv_default_loop(), req, on_work, on_after_work);
}

void launch() {
  uv_timer_t timer;
  uv_timer_init(uv_default_loop(), &timer);
  uv_timer_start(&timer, on_timer, 0, 100);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

}

CORO_EXAMPLE(UVTimerNonBlocking, uvtnb, "a nonblocking timer use libuv") {
  uv_timer_nblock::launch();
}