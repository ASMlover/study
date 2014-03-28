/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "rwlock.h"




void 
rwlock_init(RWLock* lock)
{
  lock->write = 0;
  lock->read = 0;
}

void 
rwlock_rlock(RWLock* lock)
{
  for (;;) {
    while (lock->write)
      __sync_synchronize();

    __sync_add_and_fetch(&lock->read, 1);
    if (lock->write)
      __sync_sub_and_fetch(&lock->read, 1);
    else 
      break;
  }
}

void 
rwlock_runlock(RWLock* lock)
{
  __sync_sub_and_fetch(&lock->read, 1);
}

void 
rwlock_wlock(RWLock* lock)
{
  while (__sync_lock_test_and_set(&lock->write, 1)) {
  }

  while (lock->read)
    __sync_synchronize();
}

void 
rwlock_wunlock(RWLock* lock)
{
  __sync_lock_release(&lock->write);
}
