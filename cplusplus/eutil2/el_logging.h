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
#ifndef __EL_LOGGING_HEADER_H__
#define __EL_LOGGING_HEADER_H__

namespace el {

enum class SeverityType {
  SEVERITYTYPE_DEBUG = 0, 
  SEVERITYTYPE_MESSAGE, 
  SEVERITYTYPE_WARNING, 
  SEVERITYTYPE_ERROR, 
  SEVERITYTYPE_FAIL, 
};

class Logging : public Singleton<Logging> {
  struct File {
    uint16_t year;
    uint8_t  mon;
    uint8_t  day;
    FILE*    stream;

    File(void) {
      memset(this, 0, sizeof(*this));
    }

    ~File(void) {
      if (nullptr != stream)
        fclose(stream);
    }

    bool operator==(const Time& time) const {
      return (year == time.year && mon == time.mon && day == time.day);
    }
  };
  enum {DEF_BUFSIZE = 16 * 1024};
  typedef std::unordered_map<SeverityType, File> FileMap;

  FileMap files_;
public:
  Logging(void);
  ~Logging(void);

  void Write(SeverityType severity, const char* format, ...);
  void WriteX(SeverityType severity, 
      const char* file, int line, const char* format, ...);
private:
  const char* GetSeverityName(SeverityType severity);
  FILE* GetFileStream(SeverityType severity, const Time& time);
};

}

#endif  // __EL_LOGGING_HEADER_H__
