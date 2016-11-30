// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef CHAOS_PLATFORM_H
#define CHAOS_PLATFORM_H

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_) || defined(_MSC_VER)
# define CHAOS_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__)
# define CHAOS_LINUX
#elif defined(macintosh) || defined(__APPLE__) || defined(__MACH__)
# define CHAOS_DARWIN
#else
# error "Unknown Platform."
#endif

#if !defined(CHAOS_WINDOWS)
# define CHAOS_POSIX
#endif

#define CHAOS_IMPL_WITH_STD (0)

#if defined(CHAOS_WINDOWS)
# define CHAOS_ARRAY(type, name, count) type* name = (type*)_alloca((count) * sizeof(type))
#else
# define CHAOS_ARRAY(type, name, count) type name[count]
#endif

#endif // CHAOS_PLATFORM_H
