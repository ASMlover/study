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

  void OnAllocation(std::size_t bytes);
  bool ShouldCollect() const;
  void Collect();

  const GcStats& Stats() const;
  void SetThreshold(std::size_t bytes);

 private:
  GcStats stats_;
  std::size_t threshold_bytes_;
};

}  // namespace ms

