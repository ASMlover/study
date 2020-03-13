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
  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::future<int> fut = std::async(std::launch::async, accumulate_array, arr, 0, arr.size());

  std::cout << "async_worker with accumulate: " << fut.get() << std::endl;
}

void split_async_workers() {
  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::vector<std::future<int>> futs;
  futs.push_back(std::async(std::launch::async, accumulate_array, arr, 0, arr.size() / 2));
  futs.push_back(std::async(std::launch::async, accumulate_array, arr, arr.size() / 2, arr.size() / 2));

  std::cout << "split_async_workers with accumulate: " << futs[0].get() << " | " << futs[1].get() << std::endl;
}