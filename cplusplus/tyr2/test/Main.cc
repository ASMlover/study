// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <iostream>
#include "../basic/TStringPiece.h"

#define UNUSED_PARAM(arg) (void)arg

void tyr_test_StringArg(void) {
  std::string std_str("std::string('tyr test StringArg')");
  const char* c_str = "raw c-types string('tyr test StringArg')";

  tyr::basic::StringArg s1(c_str);
  std::cout << "StringArg with raw c-types string => " << s1.c_str() << std::endl;

  tyr::basic::StringArg s2(std_str);
  std::cout << "StringArg with std::string => " << s2.c_str() << std::endl;
  tyr::basic::StringArg s3(std_str.c_str());
  std::cout << "StringArg with std::string.c_str() => " << s3.c_str() << std::endl;
  tyr::basic::StringArg s4(std_str.data());
  std::cout << "StringArg with std::string.data() => " << s4.c_str() << std::endl;
}

static void tyr_show_StringPiece(const tyr::basic::StringPiece& s, const char* name = "StringPiece") {
  const char* output = "(null)";
  if (static_cast<bool>(s))
    output = s.data();
  std::cout << "object(`" << name << "`) tyr::basic::StringPiece is: \n\t@{"
    << "'data': '" << output << "'"
    << ", 'size': " << s.size()
    << ", 'empty': " << s.empty()
    << ", 'begin': " << (void*)s.begin()
    << ", 'end': " << (void*)s.end() << "}"
    << "\n\t@as_string: " << s.as_string() << std::endl;
}

void tyr_test_StringPiece(void) {
  const char* cstr = "raw c-types string<tyr test StringPiece>";
  size_t cstr_len = strlen(cstr);
  const byte_t* bstr = reinterpret_cast<const byte_t*>("raw byte-types string<tyr test StringPiece>");
  std::string std_str("std::string<tyr test StringPiece>");
  char buf[] = "memory buffer<tyr test StringPiece>";
  size_t buf_len = sizeof(buf);

  tyr::basic::StringPiece s1;
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.set(cstr);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.set(cstr, cstr_len);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.set(buf, buf_len);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.remove_prefix(10);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  s1.remove_suffix(5);
  tyr_show_StringPiece(s1, "StringPiece.s1");
  std::cout << "@s1(" << s1 << ") starts_with<fer>: " << s1.starts_with("fer") << std::endl;
  s1.clear();
  tyr_show_StringPiece(s1, "StringPiece.s1");

  tyr::basic::StringPiece s2(bstr);
  tyr_show_StringPiece(s2, "StringPiece.s2");
  tyr::basic::StringPiece s3(std_str);
  tyr_show_StringPiece(s3, "StringPiece.s3");

  std::cout << "s2 compare with s3 -"
    << " @==: " << (s2 == s3)
    << " @<: " << (s2 < s3)
    << " @<=: " << (s2 <= s3)
    << " @>: " << (s2 > s3)
    << " @>=: " << (s2 >= s3) << std::endl;

  std::string s;
  s2.copy_to_string(&s);
  std::cout << "with copy_to_string @s: " << s << std::endl;
  s3.copy_to_string(&s);
  std::cout << "with copy_to_string @s: " << s << std::endl;
}

int main(int argc, char* argv[]) {
  UNUSED_PARAM(argc);
  UNUSED_PARAM(argv);

  tyr_test_StringArg();
  tyr_test_StringPiece();

  return 0;
}
