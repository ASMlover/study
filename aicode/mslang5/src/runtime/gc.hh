#pragma once

#include <cstddef>

namespace ms {

struct GcStats {
  std::size_t bytes_allocated = 0;
  std::size_t collections = 0;
};

class GcController {
 public:
  explicit GcController(std::size_t threshold_bytes = 1024 * 1024);

  void on_allocation(std::size_t bytes);
  bool should_collect() const;
  void collect();

  const GcStats& stats() const;
  void set_threshold(std::size_t bytes);

 private:
  GcStats stats_;
  std::size_t threshold_bytes_;
};

}  // namespace ms

