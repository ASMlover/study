// Copyright (c) 2018 ASMlover. All rights reserved.
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

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_) || \
  defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
# define _NYXCORE_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__)
# define _NYXCORE_LINUX
#elif defined(macintosh) || defined(__APPLE__) || defined(__MACH__)
# define _NYXCORE_DARWIN
#else
# error "unknown platform"
#endif

#if !defined(_NYXCORE_WINDOWS)
# define _NYXCORE_POSIX
#endif

#if defined(i386) || defined (__i386__) || defined(__i486__) || \
  defined(__i586__) || defined (__i686__) || defined(__i386) || \
  defined(_M_IX86) || defined(_X86__) || defined(__THW_INTEL__) || \
  defined(__I86__) || defined(__INTEL__)
# define _NYXCORE_ARCH32
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || \
  defined(__amd64__) || defined(_M_X64)
# define _NYXCORE_ARCH64
#else
# error "unknown architecture"
#endif
