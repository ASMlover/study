// Copyright (c) 2017 ASMlover. All rights reserved.
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
#ifndef WAKEUPSIGNAL_H_
#define WAKEUPSIGNAL_H_

#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
# include <Winsock2.h>
# include "Windows/WakeupSignal.h"

inline void sys_signal_init(void) {
  WSADATA wd;
  WSAStartup(MAKEWORD(2, 2), &wd);
}

inline void sys_signal_destroy(void) {
  WSACleanup();
}
#elif defined(__linux__)
# include "Linux/WakeupSignal.h"

# define sys_signal_init()
# define sys_signal_destroy()
#elif defined(__APPLE__) || defined(__MACH__)
# include "Darwin/WakeupSignal.h"

# define sys_signal_init()
# define sys_signal_destroy()
#endif

#endif // WAKEUPSIGNAL_H_
