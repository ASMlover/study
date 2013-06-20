//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!    notice, this list of conditions and the following disclaimer in
//!  * Redistributions in binary form must reproduce the above copyright
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
#ifndef __MINI_STL_DEFINED_HEADER_H__
#define __MINI_STL_DEFINED_HEADER_H__

#if defined(_WIN32) || defined(_WIN64)
  typedef signed __int8     int8_t;
  typedef unsigned __int8   uint8_t;
  typedef signed __int16    int16_t;
  typedef unsigned __int16  uint16_t;
  typedef signed __int32    int32_t;
  typedef unsigned __int32  uint32_t;
  typedef signed __int64    int64_t;
  typedef unsigned __int64  uint64_t;
#else
  #include <stdint.h>
#endif

#define nullptr   (0)

#endif  //! __MINI_STL_DEFINED_HEADER_H__
