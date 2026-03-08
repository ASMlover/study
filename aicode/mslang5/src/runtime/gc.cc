#include "runtime/gc.hh"

namespace ms {

GcController::GcController(const std::size_t threshold_bytes)
    : threshold_bytes_(threshold_bytes) {}

void GcController::OnAllocation(const std::size_t bytes) {
  stats_.bytes_allocated += bytes;
}

bool GcController::ShouldCollect() const {
  return stats_.bytes_allocated >= threshold_bytes_;
}

void GcController::Collect() {
  ++stats_.collections;
  stats_.bytes_allocated = 0;
}

const GcStats& GcController::Stats() const { return stats_; }

void GcController::SetThreshold(const std::size_t bytes) { threshold_bytes_ = bytes; }

}  // namespace ms

