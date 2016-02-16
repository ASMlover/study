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
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <string>

enum Token {
  TOKEN_EOF = -1,

  TOKEN_DEF = -2, // def
  TOKEN_EXT = -3, // extern

  TOKEN_ID  = -4, // identifier
  TOKEN_NUM = -5, // number
};

static std::string gIndentifier;
static double      gNumber;

static int GetToken(void) {
  static int c = ' ';

  while (isspace(c))
    c = getchar();

  if (isalpha(c)) {
    gIndentifier = c;
    while (isalnum(c = getchar()))
      gIndentifier += c;

    if (gIndentifier == "def")
      return TOKEN_DEF;
    if (gIndentifier == "extern")
      return TOKEN_EXT;
    return TOKEN_ID;
  }
  if (isdigit(c) || '.' == c) {
    std::string s;
    do {
      s += c;
      c = getchar();
    } while (isdigit(c) || '.' == c);

    gNumber = strtod(s.c_str(), 0);
    return TOKEN_NUM;
  }
  if ('#' == c) {
    do {
      c = getchar();
    } while (EOF != c && '\r' != c && '\n' != c);

    if (EOF != c)
      return GetToken();
  }
  if (EOF == c)
    return TOKEN_EOF;

  int this_char = c;
  c = getchar();
  return this_char;
}

int main(int argc, char* argv[]) {
  while (true) {
    std::cout << "ready> ";
    std::cout << GetToken() << std::endl;
  }

  return 0;
}
