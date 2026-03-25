#include "runtime/gc.hh"

#include <algorithm>
#include <cassert>

#include "runtime/object.hh"

namespace ms {

GcController::GcController(const std::size_t threshold_bytes) noexcept
    : threshold_bytes_(threshold_bytes) {
  stats_.next_gc = threshold_bytes_;
}

void GcController::register_object(RuntimeObject* object, const std::size_t bytes) noexcept {
  if (object == nullptr || bytes == 0) {
    return;
  }

  GcObjectHeader& header = object->gc_header();
  if (header.tracked) {
    return;
  }

  header.bytes = bytes;
  header.marked = false;
  header.tracked = true;
  header.next = tracked_objects_head_;
  tracked_objects_head_ = object;

  stats_.bytes_allocated += bytes;
  ++stats_.objects_tracked;
}

void GcController::mark_object(const RuntimeObject* object) noexcept {
  if (object == nullptr) {
    return;
  }
  GcObjectHeader& header = const_cast<RuntimeObject*>(object)->gc_header();
  if (!header.tracked) {
    return;
  }
  header.marked = true;
}

void GcController::register_allocation(const void* key, const std::size_t bytes) noexcept {
  if (key == nullptr || bytes == 0) {
    return;
  }
  const auto [_, inserted] = allocations_.emplace(key, AllocationEntry{bytes, false});
  if (!inserted) {
    return;
  }
  stats_.bytes_allocated += bytes;
  ++stats_.objects_tracked;
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

bool GcController::is_registered_object(const RuntimeObject* object) const noexcept {
  return object != nullptr && object->gc_header().tracked;
}

bool GcController::is_registered_allocation(const void* key) const noexcept {
  if (key == nullptr) {
    return false;
  }
  return allocations_.contains(key);
}

bool GcController::is_allocation_marked(const void* key) const noexcept {
  if (key == nullptr) {
    return false;
  }
  const auto it = allocations_.find(key);
  if (it == allocations_.end()) {
    return false;
  }
  return it->second.marked;
}
bool GcController::should_collect() const noexcept {
  return stats_.bytes_allocated >= threshold_bytes_;
}

void GcController::collect(const RootTracer& trace_roots) noexcept {
  const std::size_t previous_bytes_allocated = stats_.bytes_allocated;
  const std::size_t previous_objects_tracked = stats_.objects_tracked;

  for (RuntimeObject* object = tracked_objects_head_; object != nullptr;
       object = object->gc_header().next) {
#ifndef NDEBUG
    assert(object->gc_header().tracked &&
           "tracked object list should not contain unregistered objects");
#endif
    object->gc_header().marked = false;
  }

  for (auto& [_, entry] : allocations_) {
    entry.marked = false;
  }

  if (trace_roots) {
    trace_roots(*this);
  }

  std::size_t live_bytes = 0;
  std::size_t reclaimed_bytes = 0;
  std::size_t live_objects = 0;
  std::size_t reclaimed_objects = 0;

  sweep_object_list(&live_bytes, &reclaimed_bytes, &live_objects, &reclaimed_objects);

  for (auto it = allocations_.begin(); it != allocations_.end();) {
    if (it->second.marked) {
      live_bytes += it->second.bytes;
      ++live_objects;
      ++it;
      continue;
    }
    reclaimed_bytes += it->second.bytes;
    ++reclaimed_objects;
    it = allocations_.erase(it);
  }

#ifndef NDEBUG
  assert(live_objects + reclaimed_objects == previous_objects_tracked &&
         "GC object accounting should conserve tracked object count per cycle");
  assert(live_bytes + reclaimed_bytes == previous_bytes_allocated &&
         "GC byte accounting should conserve allocated bytes per cycle");
#endif

  ++stats_.collections;
  stats_.bytes_live = live_bytes;
  stats_.bytes_allocated = live_bytes;
  stats_.bytes_reclaimed += reclaimed_bytes;
  stats_.objects_reclaimed += reclaimed_objects;
  stats_.objects_tracked = live_objects;

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

void GcController::sweep_object_list(std::size_t* live_bytes, std::size_t* reclaimed_bytes,
                                     std::size_t* live_objects,
                                     std::size_t* reclaimed_objects) noexcept {
  RuntimeObject* prev = nullptr;
  RuntimeObject* current = tracked_objects_head_;

  while (current != nullptr) {
    GcObjectHeader& header = current->gc_header();
    RuntimeObject* next = header.next;

    if (header.marked) {
      if (live_bytes != nullptr) {
        *live_bytes += header.bytes;
      }
      if (live_objects != nullptr) {
        ++(*live_objects);
      }
      prev = current;
      current = next;
      continue;
    }

    if (reclaimed_bytes != nullptr) {
      *reclaimed_bytes += header.bytes;
    }
    if (reclaimed_objects != nullptr) {
      ++(*reclaimed_objects);
    }

    if (prev == nullptr) {
      tracked_objects_head_ = next;
    } else {
      prev->gc_header().next = next;
    }

    header.next = nullptr;
    header.bytes = 0;
    header.marked = false;
    header.tracked = false;
    current = next;
  }
}

}  // namespace ms
