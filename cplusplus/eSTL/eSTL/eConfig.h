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
#ifndef __ESTL_CONFIG_HEADER_H__
#define __ESTL_CONFIG_HEADER_H__

#ifndef ESTL_DEBUG
# if defined(_DEBUG) || defined(EVIL_DEBUG)
#   define ESTL_DEBUG 1
# else
#   define ESTL_DEBUG 0
# endif
#endif

#ifndef ESTL_DEBUGPARAMS_LEVEL
# if ESTL_DEBUG
#   define ESTL_DEBUGPARAMS_LEVEL 2
# else
#   define ESTL_DEBUGPARAMS_LEVEL 0
# endif
#endif

#ifndef ESTL_NAME_ENABLED
# define ESTL_NAME_ENABLED ESTL_DEBUG
#endif

#ifndef ESTL_NAME
# if ESTL_NAME_ENABLED
#   define ESTL_NAME(x)     x
#   define ESTL_NAME_VAL(x) x
# else
#   define ESTL_NAME(x)
#   define ESTL_NAME_VAL(x) ((const char*)nullptr)
# endif
#endif

#ifndef ESTL_DEFAULT_NAME_PREFIX
# define ESTL_DEFAULT_NAME_PREFIX "ESTL"
#endif

#ifndef ESTL_ASSERT_ENABLED
# define ESTL_ASSERT_ENABLED ESTL_DEBUG
#endif

#ifndef ESTL_EMPTY_REFERENCE_ASSERT_ENABLED
# define ESTL_EMPTY_REFERENCE_ASSERT_ENABLED ESTL_ASSERT_ENABLED
#endif

#ifndef ESTL_ASSERT_FAIL_DEFINED
# define ESTL_ASSERT_FAIL_DEFINED

namespace estl {
  typedef void (*ESTL_AssertFailFun)(const char* expression, void* context);
  
  void SetAssertFailFun(ESTL_AssertFailFun fun, void* context);
  void AssertFail(const char* expression);
  void AssertFailFunDefault(const char* expression, void* context);
}
#endif

#ifndef ESTL_ASSERT
# if ESTL_ASSERT_ENABLED
#   define ESTL_ASSERT(expr)  (void)((expr) || (estl::AssertFail(#expr), 0))
# else
#   define ESTL_ASSERT(expr)
# endif
#endif

#ifndef ESTL_FAIL_MSG
# if ESTL_ASSERT_ENABLED
#   define ESTL_FAIL_MSG(msg) (estl::AssertFail(msg))
# else
#   define ESTL_FAIL_MSG(msg)
# endif
#endif

#if !defined(ESTL_CT_ASSERT)
# if defined(ESTL_DEBUG)
    template<bool>  struct ESTL_CT_ASSERT_FAIL;
    template<>      struct ESTL_CT_ASSERT_FAIL<true> {enum {value = 1};};
    template<int x> struct ESTL_CT_ASSERT_FAIL {};

#   define ESTL_PREPROCESSOR_JOIN(a, b)   ESTL_PREPROCESSOR_JOIN1(a, b)
#   define ESTL_PREPROCESSOR_JOIN1(a, b)  ESTL_PREPROCESSOR_JOIN2(a, b)
#   define ESTL_PREPROCESSOR_JOIN2(a, b)  a##b

// TODO:
# else
#   define ESTL_CT_ASSERT(expr)
# endif
#endif

#ifndef ESTL_ALLOC_COPY_ENABLED
# define ESTL_ALLOC_COPY_ENABLED 0
#endif

#ifndef ESTL_FIXED_SIZE_TRACKING_ENABLED
# define ESTL_FIXED_SIZE_TRACKING_ENABLED ESTL_DEBUG
#endif

#ifndef ESTL_MAX_STACK_USAGE
# define ESTL_MAX_STACK_USAGE 4000
#endif

#ifndef ESTL_AddRef
# define ESTL_AddRef AddRef
#endif
#ifndef ESTL_Release
# define ESTL_Release Release
#endif

#ifndef ESTL_Alloc
# define ESTL_Alloc(allocator, n) (allocator).allocate((n))
#endif

#ifndef ESTL_AllocFlags
# define ESTL_AllocFlags(allocator, n, flags) (allocator).allocate((n), (flags))
#endif

#ifndef ESTL_AllocAligned
# define ESTL_AllocAligned(allocator, n, alignment, offset) (allocator).allocate((n), (alignment), (offset))
#endif

#ifndef ESTL_Free
# define ESTL_Free(allocator, p, size) (allocator).deallocate((p), (size))
#endif

#ifndef ESTL_AllocatorType
# define ESTL_AllocatorType estl::allocator
#endif

#ifndef ESTL_AllocatorDefault
# define ESTL_AllocatorDefault estl::GetDefaultAllocator
#endif

#endif  // __ESTL_CONFIG_HEADER_H__
