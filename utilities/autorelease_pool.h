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
#ifndef __AUTORELEASE_POOL_HEADER_H__
#define __AUTORELEASE_POOL_HEADER_H__

#include <vector>
#include <deque>

class Ref;
class AutoreleasePool : private util::UnCopyable {
  std::string       name_;
  std::vector<Ref*> objects_array_;
public:
  // Don't create an auto release pool in heap, 
  // create it in stack.
  AutoreleasePool(void);

  // Create an auto release pool with special name.
  explicit AutoreleasePool(const std::string& name);
  ~AutoreleasePool(void);

  // Add a given object into this pool.
  //
  // The same object may be added several times to the 
  // same pool, when the pool is destructed, the object's 
  // Ref::Release() method will be called for each time 
  // it was added.
  void AddObject(Ref* object);

  // Clear the auto release pool.
  //
  // Ref::Release() will be called for each time the managed 
  // objects is added to the pool.
  void Clear(void);

  // Checks whether the pool contains the specified object.
  bool Contains(Ref* object) const;
};

class PoolManager : private util::UnCopyable {
  AutoreleasePool*              current_pool_;
  std::deque<AutoreleasePool*>  pool_stack_;
public:
  static PoolManager* GetInstance(void);
  static void DestroyInstance(void);

  // Get current auto release pool, there is at least one 
  // auto release pool that created by engine. 
  // You can create your own auto release pool at demand, 
  // which will be put into auto release pool stack.
  AutoreleasePool* GetCurrentPool(void) const;

  bool IsObjectInPools(Ref* object) const;

  void Push(AutoreleasePool* pool);
  void Pop(void);
private:
  PoolManager(void);
  ~PoolManager(void);
};

#endif  // __AUTORELEASE_POOL_HEADER_H__
