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
#include "ini_parser.h"

static const std::string kNilString("");

IniParser::IniParser(void)
  : pos_(0)
  , error_(false) 
  , file_(NULL)
  , length_(0)
  , section_("") {
  memset(buffer_, 0, sizeof(buffer_));
}

IniParser::~IniParser(void) {
  Close();
}

bool IniParser::Open(const char* fname) {
  if (NULL == fname)
    return false;

  file_ = fopen(fname, "r");
  if (NULL == file_)
    return false;

  pos_     = 0;
  error_   = false;
  length_  = 0;
  section_ = "";

  return true;
}

void IniParser::Close(void) {
  if (NULL != file_) 
    fclose(file_);
}

bool IniParser::Parse(void) {
  int c = GetChar();
  while (EOF != c) {
    switch (c) {
    case '[':
      ParseSection();
      break;
    case '#':
    case ' ':
    case '\r':
    case '\t':
    case '\n':
      break;
    default:
      ParseValue();
      break;
    }

    c = GetChar();
  }

  return true;
}

std::string IniParser::Get(
    const std::string& section, const std::string& key) {
  ValueMap::iterator found(values_.find(section + key));
  if (found != values_.end())
    return (*found).second;

  return kNilString;
}

int IniParser::GetChar(void) {
  if (pos_ >= length_) {
    if (NULL != fgets(buffer_, BSIZE, file_)) {
      length_ = (int)strlen(buffer_);
      pos_ = 0;
    }
    else {
      error_ = true;
      return EOF;
    }
  }

  return buffer_[pos_++];
}

void IniParser::UngetChar(void) {
  if (!error_)
    --pos_;
}

void IniParser::ParseSection(void) {
  section_ = "";

  int c = GetChar();
  while (']' != c) {
    if (' ' == c || '\t' == c) {
    }
    else if ('\n' == c || '#' == c || EOF == c) {
      error_ = true;
      break;
    }
    else {
      section_ += (char)c;
    }

    c = GetChar();
  }

  if (error_) {
    fprintf(stderr, "Parse Section error ...\n");
    abort();
  }

  c = GetChar(); // skip ]
}

void IniParser::ParseValue(void) {
  UngetChar();
  std::string key(section_ + ParseValueKey());
  std::string value(ParseValueValue());
  values_[key] = value;
}

const std::string IniParser::ParseValueKey(void) {
  std::string key;

  bool key_begin = false;
  bool space = false;
  int c = GetChar();
  while ('=' != c) {
    if (' ' == c || '\t' == c) {
      if (key_begin)
        space = true;
    }
    else if ('\n' == c || '#' == c || EOF == c) {
      error_ = true;
      break;
    }
    else {
      if (space) {
        error_ = true;
        break;
      }

      key += (char)c;
      key_begin = true;
    }

    c = GetChar();
  }

  if (error_) {
    fprintf(stderr, "Parse Key error ... c(%d)\n", c);
    abort();
  }

  return key;
}

const std::string IniParser::ParseValueValue(void) {
  std::string value;

  bool val_begin = false;
  int c = GetChar();
  while (true) {
    if (' ' == c || '\t' == c) {
      if (val_begin)
        break;
    }
    else if ('\n' == c || '#' == c || '\r' == c) {
      break;
    }
    else if (EOF == c) {
      error_ = true;
      break;
    }
    else {
      value += (char)c;
      if (!val_begin)
        val_begin = true;
    }

    c = GetChar();
  }

  if (error_) {
    fprintf(stderr, "Parse Value error ...");
    abort();
  }

  return value;
}
