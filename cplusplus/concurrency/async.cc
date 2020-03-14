#include <iostream>
#include <numeric>
#include <future>
#include <thread>
#include <vector>

static int accumulate_array(
  const std::vector<int>& arr, std::size_t off, std::size_t n) {
  return std::accumulate(arr.begin() + off, arr.begin() + off + n, 0);
}

void async_worker() {
  std::cout << std::endl << "[" << __func__ << "] test beginning ..." << std::endl;

  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::future<int> fut = std::async(std::launch::async, accumulate_array, arr, 0, arr.size());

  std::cout << "[" << __func__ << "] with accumulate: " << fut.get() << std::endl;
}

void split_async_workers() {
  std::cout << std::endl << "[" << __func__ << "] test beginning ..." << std::endl;

  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::vector<std::future<int>> futs;
  futs.push_back(std::async(std::launch::async, accumulate_array, arr, 0, arr.size() / 2));
  futs.push_back(std::async(std::launch::async, accumulate_array, arr, arr.size() / 2, arr.size() / 2));

  std::cout << "[" << __func__ << "] with accumulate: " << futs[0].get() << " | " << futs[1].get() << std::endl;
}

static int accumulate_array_blocked(
  const std::vector<int>& arr, std::size_t off, std::size_t n) {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  return std::accumulate(arr.begin() + off, arr.begin() + off + n, 0);
}

void async_worker_timeout() {
  std::cout << std::endl << "[" << __func__ << "] test beginning ..." << std::endl;

  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::future<int> fut = std::async(std::launch::async, accumulate_array_blocked, arr, 0, arr.size());

  while (fut.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
    std::cout << "... <accumulate-blocked> still not ready" << std::endl;
  std::cout << "[" << __func__ << "] with accumulate: " << fut.get() << std::endl;
}

void async_worker_deferred() {
  std::cout << std::endl << "[" << __func__ << "] test beginning ..." << std::endl;

  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::future<int> fut = std::async(std::launch::deferred, accumulate_array_blocked, arr, 0, arr.size());

  while (fut.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
    std::cout << "... <accumulate-blocked> still not ready" << std::endl;
  std::cout << "[" << __func__ << "] with accumulate: " << fut.get() << std::endl;
}

static int accumulate_array_except(
  const std::vector<int>& arr, std::size_t off, std::size_t n) {
  throw std::runtime_error("something is broken");
  return std::accumulate(arr.begin() + off, arr.begin() + off + n, 0);
}

void async_worker_catch() {
  std::cout << std::endl << "[" << __func__ << "] test beginning ..." << std::endl;

  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  try {
    std::future<int> fut = std::async(std::launch::async, accumulate_array_except, arr, 0, arr.size());
    std::cout << "[" << __func__ << "] with accumulate: " << fut.get() << std::endl;
  }
  catch (const std::runtime_error & err) {
    std::cerr << "caught an error: " << err.what() << std::endl;
  }
}