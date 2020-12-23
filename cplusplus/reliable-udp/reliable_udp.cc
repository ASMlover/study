// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include "reliable_udp.hh"

#define GENERAL_PACKAGE (128)

namespace rudp {

struct message {
  struct message* next;
  uint8_t* buffer;
  int sz;
  int cap;
  int id;
  int tick;
};

struct message_queue {
  struct message* head;
  struct message* tail;
};

struct array {
  int cap;
  int n;
  int* a;
};

struct rudp {
  struct message_queue send_queue; // user packages will send
  struct message_queue recv_queue; // the packages received
  struct message_queue send_history; // user packages already send

  struct rudp_package* send_package; // returns by rudp_update

  struct message* free_list; // recycle message
  struct array send_argain;

  int corrupt;
  int current_tick;
  int last_send_tick;
  int last_expired_tick;
  int send_id;
  int recv_id_min;
  int recv_id_max;
  int send_delay;
  int expired;
};

}
