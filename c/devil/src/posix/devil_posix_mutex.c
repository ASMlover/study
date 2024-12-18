/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
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
#include <errno.h>
#include <stdlib.h>
#include "../devil_mutex.h"

int
devil_mutex_init(devil_mutex_t* mutex)
{
  return pthread_mutex_init(mutex, NULL);
}

void
devil_mutex_destroy(devil_mutex_t* mutex)
{
  if (0 != pthread_mutex_destroy(mutex))
    abort();
}

void
devil_mutex_lock(devil_mutex_t* mutex)
{
  if (0 != pthread_mutex_lock(mutex))
    abort();
}

int
devil_mutex_trylock(devil_mutex_t* mutex)
{
  int ret = pthread_mutex_trylock(mutex);
  if (0 != ret && EBUSY != ret && EAGAIN != ret)
    abort();

  return ret;
}

void
devil_mutex_unlock(devil_mutex_t* mutex)
{
  if (0 != pthread_mutex_unlock(mutex))
    abort();
}
