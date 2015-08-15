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

#include <codecvt>
#include <string>

inline std::wstring utf8_to_ucs2(const std::string& u8str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t> > convert;
  return convert.from_bytes(u8str);
}

inline std::wstring mb_to_ucs2(
    const std::string& mbstr, const char* loc = "") {
  typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> wcmb_type;
  std::wstring_convert<wcmb_type> convert(new wcmb_type(loc));
  return convert.from_bytes(mbstr);
}

inline std::string wc_to_mb(
    const std::wstring& wcstr, const char* loc = "") {
  typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> wcmb_type;
  std::wstring_convert<wcmb_type> convert(new wcmb_type(loc));
  return convert.to_bytes(wcstr);
}

inline std::string utf8_to_mb(
    const std::string& mbstr, const char* loc = "") {
  return wc_to_mb(utf8_to_ucs2(u8str));
}

inline std::string mb_to_utf8(
    const std::string& mbstr, const char* loc = "") {
  std::wstring_convert<std::codecvt_utf8<wchar_t> > convert;
  return convert.to_bytes(mb_to_ucs2(mbstr, loc));
}

inline std::string wc_to_utf8(const std::wstring& wcstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t> > convert;
  return convert.to_bytes(wcstr);
}
