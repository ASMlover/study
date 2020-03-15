#include <iostream>
#include <numeric>
#include <future>
#include <thread>
#include <vector>
#include "tasks.hh"

template <typename Fn, typename... Args>
inline auto really_async(Fn&& fn, Args&&... args) noexcept {
  return std::async(std::launch::async, std::forward<Fn>(fn), std::forward<Args>(args)...);
}

static int accumulate_array(
  const std::vector<int>& arr, std::size_t off, std::size_t n) {
  return std::accumulate(arr.begin() + off, arr.begin() + off + n, 0);
}

TASK_DECL(AsyncWorker) {
  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::future<int> fut = really_async(accumulate_array, arr, 0, arr.size());

  std::cout << "computing result is: " << fut.get() << std::endl;
}

TASK_DECL(AsyncWorkerSplit) {
  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::vector<std::future<int>> futs;
  futs.push_back(really_async(accumulate_array, arr, 0, arr.size() / 2));
  futs.push_back(really_async(accumulate_array, arr, arr.size() / 2, arr.size() / 2));

  std::cout << "computing result is: " << futs[0].get() << " | " << futs[1].get() << std::endl;
}

static int accumulate_array_blocked(
  const std::vector<int>& arr, std::size_t off, std::size_t n) {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  return std::accumulate(arr.begin() + off, arr.begin() + off + n, 0);
}

TASK_DECL(AsyncWorkerTimeout) {
  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::future<int> fut = really_async(accumulate_array_blocked, arr, 0, arr.size());

  while (fut.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
    std::cout << "... <accumulate-blocked> still not ready" << std::endl;
  std::cout << "computing result is: " << fut.get() << std::endl;
}

TASK_DECL(AsyncWorkerDeferred) {
  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::future<int> fut = std::async(std::launch::deferred, accumulate_array_blocked, arr, 0, arr.size());

  int counter = 0;
  while (fut.wait_for(std::chrono::seconds(1)) != std::future_status::ready) {
    std::cout << "... <accumulate-blocked> still not ready" << std::endl;

    if (++counter > 100)
      break;
  }
  std::cout << "computing result is: " << fut.get() << std::endl;
}

static int accumulate_array_except(
  const std::vector<int>& arr, std::size_t off, std::size_t n) {
  throw std::runtime_error("something is broken");
  return std::accumulate(arr.begin() + off, arr.begin() + off + n, 0);
}

TASK_DECL(AsyncWorkerCaught) {
  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  try {
    std::future<int> fut = really_async(accumulate_array_except, arr, 0, arr.size());
    std::cout << "computing result is: " << fut.get() << std::endl;
  }
  catch (const std::runtime_error & err) {
    std::cerr << "caught an error: " << err.what() << std::endl;
  }
}
