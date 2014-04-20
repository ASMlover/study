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
#ifndef __DISPATCHER_HEADER_H__
#define __DISPATCHER_HEADER_H__


#include <functional>

enum class MsgType {
  MSGTYPE_ERROR = 0, 
  MSGTYPE_LOGIN, 
  MSGTYPE_GAME, 
};

// The dispatcher of networking message.
//
// Since the Dispatcher is a singleton, the standard way 
// to use it is by calling:
//    Dispatcher::GetSingleton().methodName();
class Dispatcher : private util::UnCopyable {
  struct KeyType {
    MsgType Type;
    uint8_t Proto;

    KeyType(MsgType type, uint8_t proto) 
      : Type(type) 
      , Proto(proto) {
    }

    inline bool operator==(const KeyType& x) const {
      return (Type == x.Type && Proto == x.Proto);
    }

    inline bool operator<(const KeyType& x) const {
      return (Type < x.Type && Proto < x.Proto);
    }
  };
  typedef std::function<bool (const void*)> ValueType;

  std::map<KeyType, ValueType> dispatchers_;
public:
  Dispatcher(void);
  ~Dispatcher(void);

  // returns a shared instance of Dispatcher
  static Dispatcher& GetSingleton(void);

  // process the metworking message 
  bool Dispatch(MsgType type, uint8_t proto, const void* msg);

  // register/ungister dispatch handler
  bool RegisterDispatcher(
      MsgType type, uint8_t proto, const ValueType& callback);
  void UnregisterDispatcher(MsgType type, uint8_t proto);
};

#define DISPATCH_CALLBACK(__selector__, __target__)\
  std::bind(&__selector__, &(__target__), std::placeholders::_1)

#define REG_DISPATCHER(__type__, __proto__, __selector__, __target__)\
  Dispatcher::GetSingleton().RegisterDispatcher(\
      (__type__), (__proto__), DISPATCH_CALLBACK(__selector__, __target__))
#define UNREG_DISPATCHER(__type__, __proto__)\
  Dispatcher::GetSingleton().UnregisterDispatcher((__type__), (__target__))

#endif  // __DISPATCHER_HEADER_H__
