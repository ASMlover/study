// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __EL_WIN_SELECT_HEADER_H__
#define __EL_WIN_SELECT_HEADER_H__

namespace el {

struct win_fd_set;
struct SelectEntity;
class Select : public Poller, private UnCopyable {
  uint32_t fd_storage_;
  win_fd_set* rset_in_;
  win_fd_set* wset_in_;
  win_fd_set* rset_out_;
  win_fd_set* wset_out_;
  bool removed_;
  std::map<int, std::pair<int, Connector*> > entity_list_;
public:
  Select(void);
  ~Select(void);

  virtual bool Insert(Connector& c);
  virtual void Remove(Connector& c);
  virtual bool AddEvent(Connector& c, EventType event);
  virtual bool DelEvent(Connector& c, EventType event);
  virtual bool Dispatch(Dispatcher& dispatcher, uint32_t timeout);
private:
  bool Init(void);
  void Destroy(void);
  bool Regrow(void);
};

}

#endif  // __EL_WIN_SELECT_HEADER_H__
