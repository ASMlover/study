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
#ifndef __REF_HEADER_H__
#define __REF_HEADER_H__

class Ref {
protected:
  uint32_t ref_count_;

  Ref(void);
public:
  virtual ~Ref(void);

  // Retains the ownership. 
  //
  // This increases the Ref's reference count.
  void Retain(void);

  // Release the ownership immediately.
  //
  // This decrements the Ref's reference count.
  //
  // If the reference count reaches 0 after the decrements, 
  // the Ref is destructed.
  void Release(void);

  // Release the ownership sometime soom automatically.
  //
  // This decrements the Ref's reference count at the 
  // end of current autorelease pool block.
  //
  // If the reference count reaches 0 after the decrements, 
  // the Ref is destructed.
  Ref* Autorelease(void);

  // Returns the Ref's current reference count.
  uint32_t GetRefCount(void) const;
};

#endif  // __REF_HEADER_H__
