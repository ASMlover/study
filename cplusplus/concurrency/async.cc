#include <iostream>
#include <numeric>
#include <future>
#include <thread>
#include <vector>


void async_worker() {
  std::vector<int> arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::future<int> fut = std::async(std::launch::async,
    [](const std::vector<int>& arr) -> int {
      return std::accumulate(arr.begin(), arr.end(), 0);
    }, arr);

  std::cout << "async_worker with accumulate: " << fut.get() << std::endl;
}