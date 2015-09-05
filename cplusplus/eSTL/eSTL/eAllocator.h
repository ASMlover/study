// Copyright (c) 2015 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifndef __ESTL_ALLOCATOR_HEADER_H__
#define __ESTL_ALLOCATOR_HEADER_H__

#include <stddef.h>
#include "eConfig.h"

namespace estl {

#ifndef ESTL_ALLOC_DEFAULT_NAME
# define ESTL_ALLOC_DEFAULT_NAME ESTL_DEFAULT_NAME_PREFIX
#endif

enum AllocFlags {
  MEM_TEMP = 0,
  MEM_PERM = 1,
};

class Allocator {
#if ESTL_NAME_ENABLED
  const char* name_; // Debug name, use to track memory.
#endif
public:
  typedef size_t SizeType;

  explicit Allocator(
      const char* name = ESTL_NAME_VAL(ESTL_ALLOC_DEFAULT_NAME));
  explicit Allocator(const Allocator& x);
  Allocator(const Allocator& x, const char* name);

  Allocator& operator=(const Allocator& x);

  void* Allocate(size_t n, int flags = 0);
  void* Allocate(size_t n, size_t alignment, size_t offset, int flags = 0);
  void Deallocate(void* p, size_t n);

  const char* GetName(void) const;
  void SetName(const char* name);
};

bool operator==(const Allocator& a, const Allocator& b);
bool operator!=(const Allocator& a, const Allocator& b);

Allocator* GetDefaultAllocator(void);
Allocator* SetDefaultAllocator(Allocator* allocator);

template <typename AllocatorT>
inline AllocatorT* GetDefaultAllocator(const AllocatorT*) {
  return nullptr;
}

inline ESTL_AllocatorType* GetDefaultAllocator(const ESTL_AllocatorType*) {
  return ESTL_AllocatorDefault();
}

inline void* DefaultAllocFreeMethod(size_t n, void* buffer, void*) {
  ESTL_AllocatorType* const allocator = ESTL_AllocatorDefault();

  if (nullptr != buffer) {
    ESTL_Free(*allocator, buffer, n);
    return nullptr;
  }
  else {
    return ESTL_Alloc(*allocator, n);
  }
}

template <typename AllocatorT>
inline void* AllocateMemory(
    AllocatorT& a, size_t n, size_t alignment, size_t alignment_offset) {
  if (alignment <= 8)
    return ESTL_Alloc(a, n);
  return ESTL_AllocAligned(a, n, alignment, alignment_offset);
}

}

#ifndef ESTL_USER_DEFINED_ALLOCATOR
# include <new>

# if !ESTL_DLL
void* operator new[](size_t size, const char* name, 
    int flags, uint32_t debug_flags, const char* file, int line);
void* operator new[](size_t size, size_t alignment, 
    size_t alignment_offset, const char* name, int flags, 
    uint32_t debug_flags, const char* file, int line);
# endif

namespace estl {

inline Allocator::Allocator(const char* ESTL_NAME(name)) {
# if ESTL_NAME_ENABLED
  name_ = nullptr != name ? name : ESTL_ALLOC_DEFAULT_NAME;
# endif
}

inline Allocator::Allocator(const Allocator& ESTL_NAME(alloc)) {
# if ESTL_NAME_ENABLED
  name_ = alloc.name_;
# endif
}

inline Allocator::Allocator(const Allocator&, const char* ESTL_NAME(name)) {
# if ESTL_NAME_ENABLED
  name_ = nullptr != name ? name : ESTL_ALLOC_DEFAULT_NAME;
# endif
}

inline Allocator& Allocator::operator=(const Allocator& ESTL_NAME(alloc)) {
# if ESTL_NAME_ENABLED
  name_ = alloc.name_;
# endif

  return *this;
}

inline const char* Allocator::GetName(void) const {
# if ESTL_NAME_ENABLED
  return name_;
# else
  return ESTL_ALLOC_DEFAULT_NAME;
# endif
}

inline void Allocator::SetName(const char* ESTL_NAME(name)) {
# if ESTL_NAME_ENABLED
  name_ = name;
# endif
}

inline void* Allocator::Allocate(size_t n, int flags) {
# if ESTL_NAME_ENABLED
#   define name name_
# else
#   define name ESTL_ALLOC_DEFAULT_NAME
# endif

# if ESTL_DLL
  (void)flags;
  return ::new char[n];
# elif (ESTL_DEBUGPARAMS_LEVEL <= 0)
  return ::new((char*)0, flags, 0, (char*)0, 0) char[n];
# elif (ESTL_DEBUGPARAMS_LEVEL == 1)
  return ::new(name, flags, 0, (char*)0, 0) char[n];
# else
  return ::new(name, flags, 0, __FILE__, __LINE__) char[n];
# endif
}

inline void* Allocator::Allocate(
    size_t n, size_t alignment, size_t offset, int flags) {
# if ESTL_DLL
  ESTL_ASSERT(alignment <= 8);
  (void)alignment;
  (void)offset;
  (void)flags;
# elif (ESTL_DEBUGPARAMS_LEVEL <= 0)
  return ::new(alignment, offset, (char*)0, flags, 0, (char*)0, 0) char[n];
# elif (ESTL_DEBUGPARAMS_LEVEL == 1)
  return ::new(alignment, offset, name, flags, 0, (char*)0, 0) char[n];
# else
  return ::new(alignment, offset, name, flags, 0, __FILE__, __LINE__) char[n];
# endif

# undef name
}

inline void Allocator::Deallocate(void* p, size_t) {
  delete [] (char*)p;
}

inline bool operator==(const Allocator&, const Allocator&) {
  return true;
}

inline bool operator!=(const Allocator&, const Allocator&) {
  return false;
}

}

#endif  // ESTL_USER_DEFINED_ALLOCATOR

#endif  // __ESTL_ALLOCATOR_HEADER_H__
