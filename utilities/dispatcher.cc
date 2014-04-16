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
#include <utility.h>
#include "dispatcher.h"



Dispatcher::Dispatcher(void) {
}

Dispatcher::~Dispatcher(void) {
}

Dispatcher& Dispatcher::GetSingleton(void) {
  static Dispatcher s_shared_dispatcher;

  return s_shared_dispatcher;
}



bool Dispatcher::Dispatch(MsgType type, uint8_t proto, const void* msg) {
  KeyType key(type, proto);
  std::map<KeyType, ValueType>::iterator it = dispatchers_.find(key);

  if (it != dispatchers_.end()) 
    it->second(msg);

  return true;
}


bool Dispatcher::RegisterDispatcher(
    MsgType type, uint8_t proto, const ValueType& callback) {
  KeyType key(type, proto);
  std::map<KeyType, ValueType>::iterator it = dispatchers_.find(key);

  if (it == dispatchers_.end()) 
    dispatchers_[key] = callback;

  return true;
}

void Dispatcher::UnregisterDispatcher(MsgType type, uint8_t proto) {
  KeyType key(type, proto);
  std::map<KeyType, ValueType>::iterator it = dispatchers_.find(key);

  if (it != dispatchers_.end())
    dispatchers_.erase(it);
}
