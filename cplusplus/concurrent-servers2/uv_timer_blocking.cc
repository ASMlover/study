#include <uv.h>
#include "examples.hh"

namespace ut1 {

void on_timer(uv_timer_t* timer) {
  auto timestamp = uv_hrtime();
  std::cout << "on_timer [" << (timestamp / 1000000) % 1000000 << " ms]" << std::endl;

  if (rand() % 13 == 0) {
    std::cout << "begin sleeping ..." << std::endl;
    ::Sleep(3000);
    std::cout << "end sleep ..." << std::endl;
  }
}

void launch_timer() {
  uv_timer_t timer;
  uv_timer_init(uv_default_loop(), &timer);
  uv_timer_start(&timer, on_timer, 0, 1000);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

}

COEXT_EXAMPLE(UVTimerBlock, ut1, "A blocking timer using libuv") {
  ut1::launch_timer();
}