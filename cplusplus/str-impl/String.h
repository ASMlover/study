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
#ifndef __STRING_IMPL_HEADER_H__
#define __STRING_IMPL_HEADER_H__

#include <stdint.h>

class String {
  enum STRING_CONST {
    STRING_ALLOC_BASE = 20,
    STRING_ALLOC_GRAN = 32,
  };
  int  length_;
  char*     data_;
  int  allocted_;
  char      buffer_[STRING_ALLOC_BASE];
public:
  String(void);
  explicit String(const String& text);
  String(const String& text, int begin, int end);
  explicit String(const char* text);
  String(const char* text, int begin, int end);
  explicit String(bool b);
  explicit String(char c);
  explicit String(int i);
  explicit String(uint32_t u);
  explicit String(float f);
  ~String(void);

  int Size(void) const;
  const char* Str(void) const;
  operator const char*(void) const;
  operator const char*(void);

  char operator[](int index) const;
  char& operator[](int index);

  void operator=(const String& text);
  void operator=(const char* text);

  String operator+(const String& b);
  String operator+(const char* b);
  friend String operator+(const char* a, const String& b);
  String operator+(bool b);
  String operator+(char b);
  String operator+(int b);
  String operator+(uint32_t b);
  String operator+(float b);

  String& operator+=(const String& a);
  String& operator+=(const char* a);
  String& operator+=(bool a);
  String& operator+=(char a);
  String& operator+=(int a);
  String& operator+=(uint32_t a);
  String& operator+=(float b);

  bool operator==(const String& b) const;
  bool operator==(const char* b) const;
  friend bool operator==(const char* a, const String& b) const;

  bool operator!=(const String& b) const;
  bool operator!=(const char* b) const;
  friend bool operator!=(const char* a, cosnt String& b) const;

  int Compare(const char* text) const;
  int CompareN(const char* text, int n) const;
  int ComparePrefix(const char* text) const;

  int ICompare(const char* text) const;
  int ICompareN(const char* text, int n) const;
  int IComparePrefix(const char* text) const;

  int Length(void) const;
  int Allocated(void) const;
  void Empty(void);
  bool IsEmpty(void) const;
  void Clear(void);
  void Append(char c);
  void Append(const String& text);
  void Append(const char* text);
  void Append(const char* text, int n);
  void Insert(char c, int index);
  void Insert(const char* text, int index);
  void ToLower(void);
  void ToUpper(void);
  bool IsNumeric(void) const;
  int LengthWithoutColors(void) const;
  String& RemoveColors(void);
  void CapLength(int);
  void Fill(char c, int n);
  int Find(char c, int begin = 0, int end = -1) const;
  int Find(const char* text, 
      bool casesensitive = true, int begin = 0, int end = -1) const;
  bool Filter(const char* filter, bool casesensitive) const;
  int Last(char c) const;
  const char* Left(int len, String& result) const;
  const char* Right(int len, String& result) const;
  const char* Mid(int begin, int end, String& result) const;
  String Left(int len) const;
  String Right(int len) const;
  String Mid(int begin, int end) const;
  void StripLeading(char c);
  void StripLeading(const char* text);
  bool StripLeadingOnce(const char* text);
  void StripTrailing(char c);
  void StripTrailing(const char* text);
  bool StripTrailingOnce(const char* text);
  void Strip(char c);
  void Strip(const char* text);
  void StripTrailingWhitespace(void);
  String& StripQuotes(void);
  void Replace(const char* oldtext, const char* newtext);

  int FileNameHash(void) const;
  String& BackSlashesToSlashes(void);
  String& SetFileExtension(const char* ext);
  String& StripFileExtension(void);
  String& StripAbsoluteFileExtension(void);
  String& DefaultFileExtension(const char* ext);
  String& DefaultPath(const char* basepath);
  void AppendPath(const char* text);
  String& StripFileName(void);
  String& StripPath(void);
  void ExtractFilePath(String& dest) const;
  void ExtractFileName(String& dest) const;
  void ExtractFileBase(String& dest) const;
  void ExtractFileExtension(String& dest) const;
  bool CheckExtension(const char* text);

  int Sprintf(const char* fmt, ...);
  int Vsprintf(const char* fmt, va_list ap);
  
  void ReAllocate(int amount, bool keepold);
  void Deallocate(void);

  static int Length(const char* text);
  static char* ToLower(char* text);
  static char* ToUpper(char* text);
  static bool IsNumeric(const char* text);
  static bool IsColor(const char* text);
  static bool HasLower(const char* text);
  static bool HasUpper(const char* text);
  static int LengthWithoutColors(const char* text);
  static char* RemoveColors(char* text);
  static int Compare(const char* s1, const char* s2);
  static int CompareN(const char* s1, const char* s2, int n);
  static int ICompare(const char* s1, const char* s2);
  static int ICompareN(const char* s1, const char* s2, int n);
  static int ICompareNoColor(const char* s1, const char* s2);
  static int IComaprePath(const char* s1, const char* s2);
  static int ICompareNPath(const char* s1, const char* s2, int n);
  static void Append(char* dest, const char* src, int n);
  static void CopyNZ(char* dest, const char* sec, int destsize);
  static int SnPrintf(char* dest, int n, const char* fmt, ...);
  static int VsnPrintf(char* dest, int n, const char* fmt, va_list ap);
  static int FindChar(const char* s, 
      char c, int begin = 0, int end = -1);
  static int FindText(const char* s, const char* text, 
      bool casesensitive = true, int begin = 0, int end = -1);
  static bool Filter(
      const char* filter, const char* name, bool casesensitive);
  static void StripMediaName(const char* name, String& meidaname);
  static bool CheckExtension(const char* name, const char* ext);
  static const char* FloatArrayToString(
      const float* array, int n, int precision);

  static int Hash(const char* s);
  static int Hash(const char* s, int n);
  static int IHash(const char* s);
  static int IHash(const char* s, int n);

  static char ToLower(char c);
  static char ToUpper(char c);
  static bool CharIsPrintable(int c);
  static bool CharIsLower(int c);
  static bool CharIsUpper(int c);
  static bool CharIsAlpha(int c);
  static bool CharIsNumeric(int c);
  static bool CharIsNewLine(char c);
  static bool CharIsTab(char c);
  static int ColorIndex(int c);
private:
  void Init(void);
  void EnsureAllocted(int amount, bool keepold = true);
};

void String::EnsureAllocted(int amount, bool keepold) {
  if (amount > alloced_)
    ReAllocate(amount, keepold);
}

void String::Init(void) {
  length_   = 0;
  allocted_ = STRING_ALLOC_BASE;
  data_     = buffer_;
  data_[0]  = '\0';
  memset(buffer_, 0, sizeof(buffer_));
}

String::String(void) {
  Init();
}

String::String(const String& text) {
  Init();
  int len = text.Length();
  EnsureAllocted(len + 1);
  strcpy(data_, text.data_);
  length_ = len;
}

String::String(const char* text) {
  Init();
  if (nullptr != text) {
    int len = strlen(text);
    EnsureAllocted(len + 1);
    strcpy(data_, text);
    length_ = len;
  }
}

String::String(const char* text, int begin, int end) {
  Init();

  int len = strlen(text);
  if (end > len)
    end = len;
  if (begin > len)
    begin = len;
  else if (begin < 0)
    begin = 0;

  len = end - begin;
  if (len < 0)
    len = 0;

  EnsureAllocted(len + 1);
  for (auto i = 0; i < len; ++i)
    data_[i] = text[begin + i];

  data_[len] = '\0';
  length_ = len;
}

String::String(bool b) {
  Init();
  EnsureAllocted(2);
  data_[0] = b ? '1' : '0';
  data_[1] = '\0';
  length_ = 1;
}

String::String(char c) {
  Init();
  EnsureAllocted(2);
  data_[0] = c;
  data_[1] = '\0';
  length_ = 1;
}

String::String(int i) {
  Init();

  char text[64];
  int len = sprintf(text, "%d", i);
  EnsureAllocted(len + 1);
  strcpy(data_, text);
  length_ = len;
}

String::String(uint32_t u) {
  Init();

  char text[64];
  int len = sprintf(text, "%u", u);
  EnsureAllocted(len + 1);
  strcpy(data_, text);
  length_ = len;
}

String::String(float f) {
  Init();

  char text[64];
  int len = sprintf(text, "%f", f);
  while (len > 0 && text[len - 1] == '0')
    text[--len] = '\0';
  while (len > 0 && text[len - 1] == '.')
    text[--len] = '\0';
  EnsureAllocted(len + 1);
  strcpy(data_, text);
  length_ = len;
}

#endif  // __STRING_IMPL_HEADER_H__
