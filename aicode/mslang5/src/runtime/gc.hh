#pragma once

#include <cstddef>
#include <functional>
#include <unordered_map>

namespace ms {

struct GcStats {
  std::size_t bytes_allocated = 0;
  std::size_t bytes_live = 0;
  std::size_t bytes_reclaimed = 0;
  std::size_t collections = 0;
  std::size_t objects_tracked = 0;
  std::size_t objects_reclaimed = 0;
  std::size_t next_gc = 0;
};

class GcController {
 public:
  using RootTracer = std::function<void(GcController&)>;

  explicit GcController(std::size_t threshold_bytes = 1024 * 1024) noexcept;

  void register_allocation(const void* key, std::size_t bytes) noexcept;
  void mark_allocation(const void* key) noexcept;
  bool should_collect() const noexcept;
  void collect(const RootTracer& trace_roots) noexcept;
  void collect() noexcept;

  const GcStats& stats() const noexcept;
  void set_threshold(std::size_t bytes) noexcept;

 private:
  struct AllocationEntry {
    std::size_t bytes = 0;
    bool marked = false;
  };

  std::unordered_map<const void*, AllocationEntry> allocations_;
  GcStats stats_;
  std::size_t threshold_bytes_;
  double growth_factor_ = 2.0;
};

}  // namespace ms