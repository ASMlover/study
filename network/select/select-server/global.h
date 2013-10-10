//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#ifndef __GLOBAL_HEADER_H__
#define __GLOBAL_HEADER_H__

class WSLib {
  WSLib(const WSLib&);
  WSLib& operator =(const WSLib&);
public:
  explicit WSLib(void);
  ~WSLib(void);
};


enum LoggingType {
  LT_DEBUG = 0, 
  LT_ERROR = 1, 
  LT_FAIL  = 2, 
};
extern void LogWrite(int severity, 
    const char* file, int line, const char* format, ...);

#if defined(_MSC_VER) && (_MSC_VER < 1400)
# error "Your compiler version is too low."
#endif

#define LOG_DEBUG(fmt, ...)\
  LogWrite(LT_DEBUG, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)\
  LogWrite(LT_ERROR, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_FAIL(fmt, ...)\
  LogWrite(LT_FAIL, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)


#endif  //! __GLOBAL_HEADER_H__
