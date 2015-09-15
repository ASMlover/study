// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __EL_STRING_HEADER_H__
#define __EL_STRING_HEADER_H__

namespace el {

class String {
  struct StringData {
    const char* chars;
    int         length;
    int         hash_code;

    StringData(const char* text);
  };

  static const int FORMATTED_STRING_MAX       = 512;
  static const unsigned int EMPTY_STRING_HASH = 0x811c9dc5;
  static const char* kEmptyString;
  Ref<StringData> data_;
public:
  String(void);
  explicit String(char c);
  explicit String(const char* text);
  String(const String& other);

  bool operator<(const String& other) const;
  bool operator<=(const String& other) const;
  bool operator>(const String& other) const;
  bool operator>=(const String& other) const;
  bool operator==(const String& other) const;
  bool operator!=(const String& other) const;

  const char& operator[](int index) const;

  String operator+(const String& other) const;
  String& operator+=(const String& other);
  String operator+(char c) const;
  String& operator+=(char c);

  const char* CString(void) const;
  int Length(void) const;
  bool IsEmpty(void) const;

  int IndexOf(const String& other, int start_index = 0) const;
  String Replace(const String& from, const String& to) const;
  unsigned int HashCode(void) const;
  int Compare(const String& other) const;
  String SubString(int start_index) const;
  String SubString(int start_index, int count) const;

  double ToNumber(void) const;

  static unsigned int Fnv1Hash(const char* text);
  static String Format(const char* format, ...);
private:
  String(const String& left, const String& right);
  String(const char* text, bool on_heap);
  void Init(const char* text, bool on_heap);
};

bool operator==(const char* left, const String& right);
bool operator!=(const char* left, const String& right);
bool operator==(const String& left, const char* right);
bool operator!=(const String& left, const char* right);

std::ostream& operator<<(std::ostream& cout, const String& s);

}

#endif  // __EL_STRING_HEADER_H__
