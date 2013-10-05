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
#ifndef __EL_LOGGING_HEADER_H__
#define __EL_LOGGING_HEADER_H__

#include <stdio.h>
#include "el_config.h"


namespace el {

struct LogFile {
  uint16_t year;
  uint8_t  mon;
  uint8_t  day;
  FILE*    stream;
};


class Logging {
  enum {DEF_BUFSIZE = 16 * 1024};

  Logging(const Logging&);
  Logging& operator =(const Logging&);
public:
  enum SeverityType {
    ST_DEBUG = 0, 
    ST_MESSGAE, 
    ST_WARNING, 
    ST_ERROR, 
    ST_FAIL, 

    ST_COUNT, 
  };
private:
  LogFile file_list_[ST_COUNT];

  const char* GetSeverityName(int severity);
  FILE* GetFileStream(int severity, Time* time);
public:
  explicit Logging(void);
  ~Logging(void);

  static Logging& Singleton(void);

  void Write(int severity, const char* format, ...);
  void Write(int severity, char* file, int line, const char* format, ...);
};

}


#endif  //! __EL_LOGGING_HEADER_H__
