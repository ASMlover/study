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
#ifndef __TYR_BASIC_CONFIG_HEADER_H__
#define __TYR_BASIC_CONFIG_HEADER_H__

#if !defined(TYR_WINDOWS) || !defined(TYR_LINUX) || !defined(TYR_DARWIN)
# undef TYR_WINDOWS
# undef TYR_LINUX
# undef TYR_DARWIN
# if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS) || defined(_MSC_VER)
#   define TYR_WINDOWS
# elif defined(__linux__)
#   define TYR_LINUX
# elif defined(__APPLE__) || defined(__MACH__)
#   define TYR_DARWIN
# else
#   error ">>>>>>>>>> Unknown platform >>>>>>>>>>"
# endif
#endif

#endif // __TYR_BASIC_CONFIG_HEADER_H__
