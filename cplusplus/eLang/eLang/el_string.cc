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
#include "el_base.h"

namespace el {

const char* String::kEmptyString = "";

String::StringData::StringData(const char* text)
  : chars(text) {
  length = strlen(text);
  hash_code = Fnv1Hash(text);
}

String::String(void) {
}

String::String(char c) {
  char chars[2] = {c, '\0'};
  Init(chars, false);
}

String::String(const char* text) {
  Init(text, false);
}

String::String(const String& other)
  : data_(other.data_) {
}

bool String::operator<(const String& other) const {
  return (strcmp(CString(), other.CString()) < 0);
}

bool String::operator<=(const String& other) const {
  return (strcmp(CString(), other.CString()) <= 0);
}

bool String::operator>(const String& other) const {
  return (strcmp(CString(), other.CString()) > 0);
}

bool String::operator>=(const String& other) const {
  return (strcmp(CString(), other.CString()) >= 0);
}

bool String::operator==(const String& other) const {
  if (&other == this || data_ == other.data_)
    return true;

  if (HashCode() != other.HashCode())
    return false;
  
  return (strcmp(CString(), other.CString()) == 0);
}

bool String::operator!=(const String& other) const {
  return !(*this == other);
}

const char& String::operator[](int index) const {
  EL_ASSERT_RANGE(index, Length() + 1);
  return data_ ? data_->chars[index] : kEmptyString[0];
}

String String::operator+(const String& other) const {
  return String(*this, other);
}

String& String::operator+=(const String& other) {
  *this = *this + other;
  return *this;
}

String String::operator+(char c) const {
  return String(*this, String(c));
}

String& String::operator+=(char c) {
  *this = *this + String(c);
  return *this;
}

const char* String::CString(void) const {
  return data_ ? data_->chars : kEmptyString;
}

int String::Length(void) const {
  return data_ ? data_->length : 0;
}

bool String::IsEmpty(void) const {
  return (0 == Length());
}

int String::IndexOf(const String& other, int start_index) const {
  if (!data_)
    return -1;
  
  if (start_index >= Length())
    start_index = Length() - 1;

  const char* found = strstr(data_->chars + start_index, other.CString());
  return (nullptr != found ? static_cast<int>(found - data_->chars) : -1);
}

String String::Replace(const String& from, const String& to) const {
  String r;

  int start = 0;
  while (start < Length()) {
    int index = IndexOf(from, start);
    if (-1 != index) {
      r += SubString(start, index - start) + to;
      start = index + from.Length();
    }
    else {
      r += SubString(start, Length() - start);
      break;
    }
  }

  return std::move(r);
}

unsigned int String::HashCode(void) const {
  return data_ ? data_->hash_code : EMPTY_STRING_HASH;
}

int String::Compare(const String& other) const {
  return strcmp(CString(), other.CString());
}

String String::SubString(int start_index) const {
  if (start_index < 0)
    start_index = Length() + start_index;

  EL_ASSERT_RANGE(start_index, Length());

  int length = Length() - start_index;
  char* heap = new char[length + 1];
  strcpy(heap, CString() + start_index);

  return String(heap, true);
}

String String::SubString(int start_index, int count) const {
  if (start_index < 0)
    start_index = Length() + start_index;

  if (count < 0)
    count = Length() + count - start_index;

  EL_ASSERT_RANGE(start_index, Length());
  EL_ASSERT(start_index + count <= Length(), 
      "Range must not go past end of string.");

  char* heap = new char[count + 1];
  strncpy(heap, CString() + start_index, count);
  heap[count] = '\0';

  return String(heap, true);
}

unsigned int String::Fnv1Hash(const char* text) {
  const unsigned int fnv_prime = 0x01000193;
  const unsigned char* byte = reinterpret_cast<const unsigned char*>(text);
  unsigned int hash = EMPTY_STRING_HASH;

  while ('\0' != *byte) {
    hash *= fnv_prime;
    hash ^= static_cast<unsigned int>(*byte);
    ++byte;
  }

  return hash;
}

String String::Format(const char* format, ...) {
  char r[FORMATTED_STRING_MAX];

  va_list ap;
  va_start(ap, format);
  vsprintf(r, format, ap);
  va_end(ap);

  return std::move(String(r));
}

String::String(const String& left, const String& right) {
  int length = left.Length() + right.Length();
  char* heap = new  char[length + 1];

  strcpy(heap, left.CString());
  strcpy(heap + left.Length(), right.CString());

  Init(heap, true);
}

String::String(const char* text, bool on_heap) {
  Init(text, on_heap);
}

void String::Init(const char* text, bool on_heap) {
  if (on_heap) {
    data_ = Ref<StringData>(new StringData(text));
  }
  else {
    int length = strlen(text);
    char* heap = new char[length + 1];
    strcpy(heap, text);

    data_ = Ref<StringData>(new StringData(heap));
  }
}

bool operator==(const char* left, const String& right) {
  if (String::Fnv1Hash(left) != right.HashCode())
    return false;

  return (0 == strcmp(left, right.CString()));
}

bool operator!=(const char* left, const String& right) {
  return !(left == right);
}

bool operator==(const String& left, const char* right) {
  if (left.HashCode() != String::Fnv1Hash(right))
    return false;

  return (0 == strcmp(left.CString(), right));
}

bool operator!=(const String& left, const char* right) {
  return !(left == right);
}

std::ostream& operator<<(std::ostream& cout, const String& s) {
  cout << s.CString();
  return cout;
}

}
