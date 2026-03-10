#include "runtime/gc.hh"

namespace ms {

GcController::GcController(const std::size_t threshold_bytes)
    : threshold_bytes_(threshold_bytes) {}

void GcController::on_allocation(const std::size_t bytes) {
  stats_.bytes_allocated += bytes;
}

bool GcController::should_collect() const {
  return stats_.bytes_allocated >= threshold_bytes_;
}

void GcController::collect() {
  ++stats_.collections;
  stats_.bytes_allocated = 0;
}

const GcStats& GcController::stats() const { return stats_; }

void GcController::set_threshold(const std::size_t bytes) { threshold_bytes_ = bytes; }

}  // namespace ms

