// Copyright (c) 2019 ASMlover. All rights reserved.
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
#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace wrencc {

using nil_t   = std::nullptr_t;
using byte_t  = std::uint8_t;
using i8_t    = std::int8_t;
using u8_t    = std::uint8_t;
using i16_t   = std::int16_t;
using u16_t   = std::uint16_t;
using i32_t   = std::int32_t;
using u32_t   = std::uint32_t;
using i64_t   = std::int64_t;
using u64_t   = std::uint64_t;
using ssz_t   = std::int32_t;
using sz_t    = std::size_t;
using str_t   = std::string;
using strv_t  = std::string_view;

class Copyable {
protected:
  Copyable(void) noexcept = default;
  ~Copyable(void) noexcept = default;
  Copyable(const Copyable&) noexcept = default;
  Copyable(Copyable&&) noexcept = default;
  Copyable& operator=(const Copyable&) noexcept = default;
  Copyable& operator=(Copyable&&) noexcept = default;
};

class UnCopyable {
  UnCopyable(const UnCopyable&) noexcept = delete;
  UnCopyable(UnCopyable&&) noexcept = delete;
  UnCopyable& operator=(const UnCopyable&) noexcept = delete;
  UnCopyable& operator=(UnCopyable&&) noexcept = delete;
protected:
  UnCopyable(void) noexcept = default;
  ~UnCopyable(void) noexcept = default;
};

namespace Xt {
  template <typename T, typename U>
  inline T as_type(U x) { return static_cast<T>(x); }

  template <typename T, typename U>
  inline T* cast(U* x) { return static_cast<T*>(x); }

  template <typename T, typename U>
  inline T* down(U* x) { return dynamic_cast<T*>(x); }

  inline str_t to_string(double d) {
    std::stringstream ss;
    ss << std::setprecision(std::numeric_limits<double>::max_digits10) << d;
    return ss.str();
  }

  inline int power_of_2ceil(int n) {
    // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    ++n;

    return n;
  }
}

}

// if true then Wren will use a NaN tagged double for its core value
// representation, otherwise it will use a larger more conventional struct,
// the former is significantly faster and more compact, the latter is useful
// for debugging and may be more protable
//
// Defaults to on
#ifndef NAN_TAGGING
# define NAN_TAGGING        (true)
#endif

// if true the VM's interpreter loop use computed gotos, see this for more:
// https://gcc.gnu.org/onlinedocs/gcc-3.1.1/gcc/Labels-as-Values.html, enabling
// this speeds up the main dispatch loop a bit, but requires compiler support
//
// Defaults to on if not MSVC
#ifndef COMPUTED_GOTOS
# ifndef _MSC_VER
#   define COMPUTED_GOTOS   (true)
# else
#   define COMPUTED_GOTOS   (false)
# endif
#endif

// if true loads the `IO` class in the standard library
//
// Defaults to on
#ifndef USE_LIBIO
# define USE_LIBIO          (true)
#endif

// the VM includes a number of optional `auxiliary` modules, you can choose to
// include these or not, by default, they are all available, to disable one,
// set the corresponding `AUX_<name>` define to `false`
//
// Defaults to on
#ifndef AUX_META
# define AUX_META           (true)
#endif

#ifndef AUX_RANDOM
# define AUX_RANDOM         (true)
#endif

// set this to true to stress test the GC, it will perform a collection before
// every allocation, this is useful to ensure that memory is always correctly
// pinned
#define GC_STRESS           (false)

// set this to true to log memory operations as they occured
#define TRACE_MEMORY        (false)

// set this to true to log garbage collections as the occured
#define TRACE_GC            (false)

// set this to true to print out the compiled bytecode of each function
#define DUMP_COMPILED       (false)

// set this to trace each instructions as it's executed
#define TRACE_INSTRUCTIONS  (false)

// set this to display object's detail information
#define TRACE_OBJECT_DETAIL (false)

// the maximum number of module-level variables that may be defined at one
// time, this limitation comes from the 16-bits used for the arguments to
// `Code::LOAD_MODULE_VAR` and `Code::STORE_MODULE_VAR`
#define MAX_MODULE_VARS     (65536)

// the maximum number of arguments that can be passed to a method, note that
// this limitation is hardcoded in order places in the VM, in particular the
// `Code::CALL_*` instructions assume a certain maximum number
#define MAX_PARAMETERS      (16)

// the maximum name of a method, not including the signature, this is an
// arbitrary but enforced maximum just so we know how long the method name
// strings need to be in the parser
#define MAX_METHOD_NAME     (64)

// the maximum length of a method signature, signatures looks like:
//
//    foo         // getter
//    foo()       // no argument method
//    foo(_)      // one argument method
//    foo(_,_)    // two arguments method
//    init foo()  // constructor initializer
//
// the maximum signature length takes into account the longest method name,
// the maximum number of parameters with separators between them, "init" and "()"
#define MAX_METHOD_SIGNATURE  (MAX_METHOD_NAME + (MAX_PARAMETERS * 2) + 6)

// the maximum length of an identifier, the only real reason for this limitation
// is so that error messages mentioning variables can be stack allocated
#define MAX_VARIABLE_NAME   (64)

// the maximum number of fields a class can have, including inherited fields,
// this is explicit in the bytecode since `Code::CLASS` and `Code::SUBCLASS`
// take a single byte for the number of fields, note that it's 255 and not
// 256 because creating a class takes the *number* of fields, not the *highest
// field index*
#define MAX_FIELDS          (255)

#ifndef NDEBUG
# define ASSERT(cond, msg) do {\
  if (!(cond)) {\
    std::cerr << "[" << __FILE__ << ": " << __LINE__ << "] "\
              << "Assert failed in " << __func__ << "(): "\
              << msg << std::endl;\
    std::abort();\
  }\
} while (false)

// indicates that we know execution should never reach this point in the
// program, in debug mode we assert this fact because it's a bug to get here
//
// in release mode, we use compiler-specific built in functions to tell the
// compiler the code can't be reached, this avoids `missing return` warnings
// in some cases and also lets it perform some optimizations by assuming the
// code in never reached
#define UNREACHABLE() do {\
  std::cerr << "[" << __FILE__ << ": " << __LINE__ << "] "\
            << "This code should not be reached in " << __func__ << "()"\
            << std::endl;\
  std::abort();\
} while (false)
#else
# define ASSERT(cond, msg)  ((void)0)
# if defined(_MSC_VER)
#   define UNREACHABLE()    __assume(0)
# elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
#   define UNREACHABLE()    __builtin_unreachable()
# else
#   define UNREACHABLE()
# endif
#endif
