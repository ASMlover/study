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
protected:
  struct StringRecord {
    uint32_t ref_count;
    uint32_t length;
    char     data[1];
  };

  StringRecord* str_record_;

  static StringRecord* GetNullStringRecord(void) {
    static StringRecord null_string = {0x7fffffff, 0, {0}};
    return &null_string;
  }

  void DecRef(void) {
    if (!--str_record_->ref_count && str_record_ != GetNullStringRecord())
      free(str_record_);
  }
public:
  String(void);
  ~String(void);
  explicit String(const String& text);
  explicit String(const char* text);
  String(const char* text, uint32_t length);
  String(const uint8_t* data, uint32_t length);

  String& operator=(const String& text);
  String& operator=(const char* text);
  String& operator=(const char* text, uint32_t length);
  String& operator=(const uint8_t* data, uint32_t length);

  String operator+(const String& text);
  String operator+(const char* text);
  String operator+(const char* text, uint32_t length);
  String operator+(const uint8_t* data, uint32_t length);

  String& operator+=(const String& text);
  String& operator+=(const char* text);
  String& operator+=(const char* text, uint32_t length);
  String& operator+=(const uint8_t* data, uint32_t length);

  bool operator==(const String& text) const;
  bool operator==(const char* text) const;
  bool operator==(const char* text, uint32_t length) = const;
  bool operator==(const uint8_t* data, uint32_t length) = const;

  void Set(const char* text);
  void Set(const char* text, uint32_t length);
  void Set(const uint8_t* data, uint32_t length);
  void Reserve(uint32_t length);

  operator const char*(void) const;
  operator char*(void);
  const char* c_str(void) const;
  const uint8_t* data(void) const;
  uint8_t* data(void);
  uint32_t Length(void) const;
  bool IsEmpty(void) const;
  void Clear(void);
};

String::String(void) {
  str_record_ = GetNullStringRecord();
  ++str_record_->ref_count;
}

String::~String(void) {
  DecRef();
}

String::String(const String& text) {
  str_record_ = text.str_record_;
  ++str_record_->ref_count;
}

String::String(const char* text) {
  str_record_ = GetNullStringRecord();
  ++str_record_->ref_count;
  Set(text);
}

String::String(const char* text, uint32_t length) {
  str_record_ = GetNullStringRecord();
  ++str_record_->ref_count;
  Set(text, length);
}

String::String(const uint8_t* data, uint32_t length) {
  str_record_ = GetNullStringRecord();
  ++str_record_->ref_count;
  Set(data, length);
}

void String::Set(const char* text) {
  uint32_t length = strlen(text);
  Set(text, length);
}

void String::Set(const char* text, uint32_t length) {
  Reserve(length);
  memcpy(str_record_->data, text, length);
}

void String::Set(const uint8_t* data, uint32_t length) {
  Set(static_cast<const char*>(data), length);
}

void String::Reserve(uint32_t length) {
  DecRef();
  str_record_ = (StringRecord*)malloc(sizeof(StringRecord) + length);
  str_record_->length = length;
  str_record_->data[length] = 0;
  ++str_record_->ref_count;
}

String::operator const char*(void) const {
  return str_record_->data;
}

String::operator char*(void) {
  return str_record_->data;
}

const char* String::c_str(void) const {
  return str_record_->data;
}

const uint8_t* String::data(void) const {
  return static_cast<uint8_t*>(str_record_->data);
}

uint8_t* String::data(void) {
  return static_cast<uint8_t*>(str_record_->data);
}

uint32_t String::Length(void) const {
  return str_record_->length;
}

bool String::IsEmpty(void) const {
  return (str_record_ == GetNullStringRecord());
}

void String::Clear(void) {
  Reserve(0);
}

#endif  // __STRING_IMPL_HEADER_H__
