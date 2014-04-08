// Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __STATIC_ASSERT_HEADER_H__
#define __STATIC_ASSERT_HEADER_H__

namespace util {

// use nonzero array to implement
//
// #define STATIC_ASSERT(expr) do {\
//   char __assert_array[(expr) ? 1 : 0];\
//   __assert_array;\
// } while (0)


// implemention with C++11
// #define STATIC_ASSERT(expr) do {\
//   enum class Checker {
//     CHECKER_VALUE = 1 / static_cast<int>(expr), 
//   };
// } while (0)



// use incomplete type to implement 
//
// template <bool expr> struct CompileTimeChecker;
// template <> struct CompileTimeChecker<true> {};
// #define STATIC_ASSERT(expr) do {\
//   CompileTimeChecker<(expr)>();\
// } while (0)


// use sizeof to implement 
//
// template <bool expr> struct CompileTimeChecker;
// template <> struct CompileTimeChecker<true> {};
// #define STATIC_ASSERT(expr) do {\
//   (void)sizeof(CompileTimeChecker<(expr)>);\
// } while (0)



// use strongly enumerates with C++11
//
template <bool expr> struct CompileTimeChecker;
template <> struct CompileTimeChecker<true> {
  enum class Checker : bool {
    CHECKER_VALUE = true, 
  };
};
#define STATIC_ASSERT(expr) do {\
  (void)CompileTimeChecker<(expr)>::Checker::CHECKER_VALUE;\
} while (0)


}

#endif  // __STATIC_ASSERT_HEADER_H__
