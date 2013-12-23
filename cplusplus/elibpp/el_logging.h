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


#if defined(PLATFORM_WIN)
# if defined(_MSC_VER) && (_MSC_VER < 1400)
#   error "Your compiler version is too low"
# endif
#endif


namespace el {

struct LogFile {
  uint16_t year;
  uint8_t  mon;
  uint8_t  day;
  FILE*    stream;
};


struct Time;
class Logging : public Singleton<Logging>, private NonCopyable {
  enum {kDefBufferSize = 16 * 1024};
public:
  enum SeverityType {
    kSeverityTypeDebug = 0, 
    kSeverityTypeMessage, 
    kSeverityTypeWarning, 
    kSeverityTypeError, 
    kSeverityTypeFail, 

    kSeverityTypeCount, 
  };
private:
  LogFile file_list_[kSeverityTypeCount];

  const char* GetSeverityName(int severity);
  FILE* GetFileStream(int severity, Time* time);
public:
  explicit Logging(void);
  ~Logging(void);

  void Write(int severity, const char* format, ...);
  void WriteX(int severity, 
      const char* file, int line, const char* format, ...);
};

}

#define LOG_DEBUG(fmt, ...)\
  el::Logging::Instance().Write(el::Logging::kSeverityTypeDebug, \
      (fmt), ##__VA_ARGS__)
#define LOG_MSG(fmt, ...)\
  el::Logging::Instance().Write(el::Logging::kSeverityTypeMessage, \
      (fmt), ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)\
  el::Logging::Instance().Write(el::Logging::kSeverityTypeWarning, \
      (fmt), ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)\
  el::Logging::Instance().Write(el::Logging::kSeverityTypeError, \
      (fmt), ##__VA_ARGS__)
#define LOG_FAIL(fmt, ...)\
  el::Logging::Instance().Write(el::Logging::kSeverityTypeFail, \
      (fmt), ##__VA_ARGS__)

#define LOG_DEBUGX(fmt, ...)\
  el::Logging::Instance().WriteX(el::Logging::kSeverityTypeDebug, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_MSGX(fmt, ...)\
  el::Logging::Instance().WriteX(el::Logging::kSeverityTypeMessage, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_WARNX(fmt, ...)\
  el::Logging::Instance().WriteX(el::Logging::kSeverityTypeWarning, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_ERRX(fmt, ...)\
  el::Logging::Instance().WriteX(el::Logging::kSeverityTypeError, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOG_FAILX(fmt, ...)\
  el::Logging::Instance().WriteX(el::Logging::kSeverityTypeFail, \
      __FILE__, __LINE__, (fmt), ##__VA_ARGS__)


#endif  //! __EL_LOGGING_HEADER_H__
