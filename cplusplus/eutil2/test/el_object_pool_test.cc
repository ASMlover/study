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
#include "../el_unit.h"

class ObjectSample : private el::UnCopyable {
  std::string name_;
  int         id_;
public:
  explicit ObjectSample(int id = 0) 
    : name_("ObjectSample") 
    , id_(id) {
    UNIT_PRINT("%s\n", __func__);
  }

  ~ObjectSample(void) {
    UNIT_PRINT("%s\n", __func__);
  }

  inline void SetID(int id) {
    id_ = id;
  }
  
  inline void Show(void) {
    UNIT_PRINT("%s : %d\n", name_.c_str(), id_);
  }
};

UNIT_IMPL(ObjectPool) {
  el::ObjectPool<ObjectSample> pools;

  ObjectSample& obj1 = pools.AcquireObject();
  obj1.SetID(1);
  obj1.Show();

  ObjectSample& obj2 = pools.AcquireObject();
  obj2.SetID(2);
  obj2.Show();

  pools.ReleaseObject(obj1);
  pools.ReleaseObject(obj2);
}
