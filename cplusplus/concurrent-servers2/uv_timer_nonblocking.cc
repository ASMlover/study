#include <uv.h>
#include "examples.hh"

namespace ut2 {

void on_work(uv_work_t* work_req) {
  if (rand() % 13 == 0) {
    std::cout << "on_work - sleeping ..." << std::endl;
    ::Sleep(3000);
  }
}

void on_after_work(uv_work_t* work_req, int status) {
  free(work_req);
}

void on_timer(uv_timer_t* timer) {
  auto timestamp = uv_hrtime();
  std::cout << "on_timer [" << (timestamp / 1000000) % 1000000 << " ms]" << std::endl;

  uv_work_t* work_req = (uv_work_t*)malloc(sizeof(*work_req));
  uv_queue_work(uv_default_loop(), work_req, on_work, on_after_work);
}

void launch_timer() {
  uv_timer_t timer;
  uv_timer_init(uv_default_loop(), &timer);
  uv_timer_start(&timer, on_timer, 0, 1000);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

}

COEXT_EXAMPLE(UVTimerNonBlock, ut2, "A nonblocking timer using libuv") {
  ut2::launch_timer();
}