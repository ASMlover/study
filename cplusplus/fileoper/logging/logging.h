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

#include <stdio.h>
#include <map>

class Logging {
  enum {DEF_BUFSIZE = 16 * 1024};
  std::map<int, FILE*> file_list_;

  Logging(const Logging&);
  Logging& operator =(const Logging&);
public:
  enum SeverityType {
    SEVERITY_DEBUG    = 0, 
    SEVERITY_MESSAGE  = 1, 
    SEVERITY_WARNING  = 2, 
    SEVERITY_ERROR    = 3, 
  };

  Logging(void);
  ~Logging(void);

  static Logging& Singleton(void);
  int Write(int severity, const char* file, 
      const char* function, int line, const char* format, ...);
};

#define LogWrite(severity, format, ...)\
  Logging::Singleton().Write((severity), \
      __FILE__, \
      __FUNCTION__, \
      __LINE__, \
      (format), \
      __VA_ARGS__)


#endif  //! __LOGGING_HEADER_H__
