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
#ifndef __LOGGING_HEADER_H__
#define __LOGGING_HEADER_H__

class Logging {
  Logging(const Logging&);
  Logging& operator =(const Logging&);
public:
  explicit Logging(void);
  ~Logging(void);

  static Logging& Singleton(void);

  enum LoggingType {
    kLoggingTypeDebug = 0, 
    kLoggingTypeMessage, 
    kLoggingTypeWarning, 
    kLoggingTypeError, 
    kLoggingTypeFail, 
  };
  void Write(int type, 
      const char* file, 
      int line, 
      const char* format, ...);
};


#if defined(_MSC_VER) && (_MSC_VER < 1400)
# error "Your compiler it too low."
#endif


#define LOG_DBG(fmt, ...)\
  Logging::Singleton().Write(Logging::kLoggingTypeDebug, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_MSG(fmt, ...)\
  Logging::Singleton().Write(Logging::kLoggingTypeMessage, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)\
  Logging::Singleton().Write(Logging::kLoggingTypeWarning, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)\
  Logging::Singleton().Write(Logging::kLoggingTypeError, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_FAIL(fmt, ...)\
  Logging::Singleton().Write(Logging::kLoggingTypeFail, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)

#endif  //! __LOGGING_HEADER_H__
