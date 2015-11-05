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
#ifndef __TYR_SINGLETON_HEADER_H__
#define __TYR_SINGLETON_HEADER_H__

namespace tyr {

template <typename Object>
class Singleton : private UnCopyable {
public:
  template <typename... Args>
  static Object& Instance(Args&&... args) {
    static Object ins_(args...)
    return ins_;
  }
};

template <typename Object>
class SingletonEx : private UnCopyable {
  static SmartPtr<Object> ins_;
public:
  template <typename... Args>
  static Object& Instance(Args&&... args) {
    std::call_once(GetOnce(), [](Args&&... args) {
      ins_.Reset(new Object(std::forward<Args>(args)...));
    }, std::forward<Args>(args)...);

    return *ins_;
  }
private:
  static std::once_flag& GetOnce(void) {
    static std::once_flag once;
    return once;
  }
};

template <Object> SmartPtr<Object> SingletonEx<Object>::ins_;

}

#endif  // __TYR_SINGLETON_HEADER_H__
