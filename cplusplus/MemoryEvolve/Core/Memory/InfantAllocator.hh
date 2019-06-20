#pragma once

#include <vector>
#include <Core/MemoryEvolve.hh>

namespace _mevo {

struct MemoryBlock;

class InfantAllocator final : private UnCopyable {
  static constexpr sz_t kAlign = 8;
  static constexpr sz_t kMaxBytes = 1 << 12;
  static constexpr sz_t kNumber = 64;
  static constexpr sz_t kFreelist = kMaxBytes / kAlign;

  MemoryBlock* freelist_[kFreelist]{};
  std::vector<MemoryBlock*> chunks_;

  inline sz_t as_index(sz_t bytes) const {
    return (bytes + kAlign - 1) / kAlign - 1;
  }

  InfantAllocator(void) noexcept;
  ~InfantAllocator(void) noexcept;

  MemoryBlock* alloc_chunk(sz_t index);
public:
  static InfantAllocator& instance(void) {
    static InfantAllocator ins;
    return ins;
  }

  void* alloc(sz_t n);
  void dealloc(void* p, sz_t n);
};

}
