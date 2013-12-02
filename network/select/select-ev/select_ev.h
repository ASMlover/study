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
#ifndef __SELECT_EVENT_HEADER_H__
#define __SELECT_EVENT_HEADER_H__

#if defined(_WINDOWS_) || defined(_MSC_VER)
# define EV_WIN
#elif defined(__linux__)
# define EV_POSIX
#endif 


#if defined(EV_WIN)
# if defined(_MSC_VER) && (_MSC_VER < 1600)
#   if (_MSC_VER < 1300)
      typedef signed char       int8_t;
      typedef unsigned char     uint8_t;
      typedef signed short      int16_t;
      typedef unsigned short    uint16_t;
      typedef signed int        int32_t;
      typedef unsigned int      uint32_t;
#   else 
      typedef signed __int8     int8_t;
      typedef unsigned __int8   uint8_t;
      typedef signed __int16    int16_t;
      typedef unsigned __int16  uint16_t;
      typedef signed __int32    int32_t;
      typedef unsigned __int32  uint32_t;
#   endif
    typedef signed __int64      int64_t;
    typedef unsigned __int64    uint64_t;
# else
#   include <stdint.h>
# endif
#elif defined(EV_POSIX)
# include <stdint.h>
#endif 


enum NetType {
  kNetTypeInval = -1, 
  kNetTypeError = -1, 
};

enum EventType {
  kEventTypeUnknown = 0, 
  kEventTypeRead    = 1, 
  kEventTypeWrite   = 2, 
};


class Socket;
struct EventHandler {
  virtual ~EventHandler(void);
  virtual bool AcceptEvent(Socket* s);
  virtual void CloseEvent(Socket* s);
  virtual bool ReadEvent(Socket* s);
};


class EventDispatcher;
struct EventPoll {
  virtual ~EventPoll(void) {}
  virtual bool Insert(int fd, Socket* s) = 0;
  virtual void Remove(int fd) = 0;
  virtual bool AddEvent(int fd, int ev) = 0;
  virtual bool DelEvent(int fd, int ev) = 0;
  virtual bool Dispatch(EventDispatcher* dispatcher, int millitm) = 0;
};


class EventDispatcher {
  EventHandler* handler_;
  EventPoll* poll_;

  EventDispatcher(const EventDispatcher&);
  EventDispatcher& operator =(const EventDispatcher&);
public:
  explicit EventDispatcher(void);
  ~EventDispatcher(void);

  inline void Attach(EventHandler* handler)
  {
    handler_ = handler;
  }

  inline void Attach(EventPoll* poll)
  {
    poll_ = poll;
  }
public:
  bool DispatchReader(Socket* s);
  bool DispatchWriter(Socket* s);
};

#endif  //! __SELECT_EVENT_HEADER_H__
