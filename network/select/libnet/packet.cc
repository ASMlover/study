//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include <stdlib.h>
#include <string.h>
#include "logging.h"
#include "buffer.h"
#include "packet.h"



Packet::Packet(void)
  : alloced_(false)
  , data_(NULL)
{
  memset(&header_, 0, sizeof(header_));
  header_.signature = LIBNET_SIGNATURE;
}

Packet::~Packet(void)
{
  ReleaseData();
}

bool 
Packet::Encode(Buffer* dest)
{
  if (NULL == dest || NULL == data_ || header_.size <= 0)
    return false;

  dest->Put((const char*)&header_, sizeof(header_));
  dest->Put((const char*)data_, header_.size);

  return true;
}

bool 
Packet::Decode(Buffer* src)
{
  if (NULL == src || alloced_)
    return false;

  int header_len = sizeof(PacketHeader);
  if (src->length() < header_len)
    return false;
  
  PacketHeader* header = (PacketHeader*)src->buffer();
  if (LIBNET_SIGNATURE != header->signature)
    return false;
  if (src->length() < (header_len + header->size))
    return false;

  data_ = (char*)malloc(header->size);
  if (NULL == data_)
    return false;
  alloced_ = true;

  src->Get(header_len, (char*)&header_);
  src->Get(header_.size, data_);

  return true;
}


void 
Packet::ReleaseData(void)
{
  if (NULL != data_ && alloced_) {
    free(data_);
    data_ = NULL;

    alloced_ = false;
  }
}
