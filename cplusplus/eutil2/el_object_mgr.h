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
#ifndef __EL_OBJECT_MGR_HEADER_H__
#define __EL_OBJECT_MGR_HEADER_H__

namespace el {

template <typename Key, typename Object, typename Locker = DummyLock>
class ObjectMgr : private UnCopyable {
  typedef typename std::shared_ptr<Object>            ObjectPtr;
  typedef typename std::unordered_map<Key, ObjectPtr> ObjectMap;
  typedef typename ObjectMap::iterator                ObjectIter;

  Locker    locker_;
  ObjectMap objects_;
public:
  inline bool InsertObject(const Key& key, ObjectPtr obj) {
    LockerGuard<Locker> guard(locker_);

    if (objects_.find(key) != objects_.end())
      return false;

    objects_[key] = obj;
    return true;
  }

  inline void RemoveObject(const Key& key) {
    LockerGuard<Locker> guard(locker_);

    objects_.erase(key);
  }

  inline ObjectPtr GetObject(const Key& key) {
    LockerGuard<Locker> guard(locker_);

    ObjectIter it(objects_.find(key));
    if (it == objects_.end())
      return ObjectPtr(nullptr);

    return (*it).second;
  }

  inline void Clear(void) {
    LockerGuard<Locker> guard(locker_);

    objects_.clear();
  }

  inline uint32_t Size(void) const {
    return static_cast<uint32_t>(objects_.size());
  }
};

}

#endif  // __EL_OBJECT_MGR_HEADER_H__
