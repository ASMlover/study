#include "runtime/gc.hh"

#include <algorithm>

namespace ms {

GcController::GcController(const std::size_t threshold_bytes) noexcept
    : threshold_bytes_(threshold_bytes) {
  stats_.next_gc = threshold_bytes_;
}

void GcController::register_allocation(const void* key, const std::size_t bytes) noexcept {
  if (key == nullptr || bytes == 0) {
    return;
  }
  const auto [it, inserted] = allocations_.emplace(key, AllocationEntry{bytes, false});
  if (!inserted) {
    return;
  }
  stats_.bytes_allocated += bytes;
  stats_.objects_tracked = allocations_.size();
}

void GcController::mark_allocation(const void* key) noexcept {
  if (key == nullptr) {
    return;
  }
  const auto it = allocations_.find(key);
  if (it == allocations_.end()) {
    return;
  }
  it->second.marked = true;
}

bool GcController::should_collect() const noexcept {
  return stats_.bytes_allocated >= threshold_bytes_;
}

void GcController::collect(const RootTracer& trace_roots) noexcept {
  for (auto& [_, entry] : allocations_) {
    entry.marked = false;
  }

  if (trace_roots) {
    trace_roots(*this);
  }

  std::size_t live_bytes = 0;
  std::size_t reclaimed_bytes = 0;
  std::size_t reclaimed_objects = 0;
  for (auto it = allocations_.begin(); it != allocations_.end();) {
    if (it->second.marked) {
      live_bytes += it->second.bytes;
      ++it;
      continue;
    }
    reclaimed_bytes += it->second.bytes;
    ++reclaimed_objects;
    it = allocations_.erase(it);
  }

  ++stats_.collections;
  stats_.bytes_live = live_bytes;
  stats_.bytes_allocated = live_bytes;
  stats_.bytes_reclaimed += reclaimed_bytes;
  stats_.objects_reclaimed += reclaimed_objects;
  stats_.objects_tracked = allocations_.size();

  const std::size_t suggested = static_cast<std::size_t>(
      static_cast<double>(std::max<std::size_t>(live_bytes, 1024)) * growth_factor_);
  threshold_bytes_ = std::max<std::size_t>(suggested, 1024);
  stats_.next_gc = threshold_bytes_;
}

void GcController::collect() noexcept { collect(RootTracer{}); }

const GcStats& GcController::stats() const noexcept { return stats_; }

void GcController::set_threshold(const std::size_t bytes) noexcept {
  threshold_bytes_ = bytes;
  stats_.next_gc = threshold_bytes_;
}

}  // namespace ms