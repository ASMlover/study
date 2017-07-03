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
#include "ref.h"
#include "autorelease_pool.h"

AutoreleasePool::AutoreleasePool(void)
  : name_("") {
  objects_array_.reserve(150);
  PoolManager::GetInstance()->Push(this);
}

AutoreleasePool::AutoreleasePool(const std::string& name)
  : name_(name) {
  objects_array_.reserve(150);
  PoolManager::GetInstance()->Push(this);
}

AutoreleasePool::~AutoreleasePool(void) {
  Clear();
  PoolManager::GetInstance()->Pop();
}

void AutoreleasePool::AddObject(Ref* object) {
  objects_array_.push_back(object);
}

void AutoreleasePool::Clear(void) {
  for (auto* obj : objects_array_)
    obj->Release();

  objects_array_.clear();
}

bool AutoreleasePool::Contains(Ref* object) const {
  for (auto* obj : objects_array_) {
    if (object == obj)
      return true;
  }

  return false;
}

static PoolManager* s_sharedPoolManager = nullptr;
PoolManager* PoolManager::GetInstance(void) {
  if (nullptr == s_sharedPoolManager) {
    s_sharedPoolManager = new PoolManager();
    s_sharedPoolManager->current_pool_
        = new AutoreleasePool("utility auto release pool");
    s_sharedPoolManager->pool_stack_.push_back(
        s_sharedPoolManager->current_pool_);
  }

  return s_sharedPoolManager;
}

void PoolManager::DestroyInstance(void) {
  if (nullptr != s_sharedPoolManager) {
    delete s_sharedPoolManager;
    s_sharedPoolManager = nullptr;
  }
}

PoolManager::~PoolManager(void) {
  for (auto* pool : pool_stack_)
    delete pool;

  pool_stack_.clear();
}

AutoreleasePool* PoolManager::GetCurrentPool(void) const {
  return current_pool_;
}

bool PoolManager::IsObjectInPools(Ref* object) const {
  for (const auto& pool : pool_stack_) {
    if (pool->Contains(object))
      return true;
  }

  return false;
}

void PoolManager::Push(AutoreleasePool* pool) {
  pool_stack_.push_back(pool);
  current_pool_ = pool;
}

void PoolManager::Pop(void) {
  if (!pool_stack_.empty())
    pool_stack_.pop_back();

  if (pool_stack_.size() > 1)
    current_pool_ = pool_stack_.back();
}
