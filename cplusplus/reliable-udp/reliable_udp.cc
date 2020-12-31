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
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include "reliable_udp.hh"

#define GENERAL_PACKAGE (128)

#define TYPE_IGNORE     (0)
#define TYPE_CORRUPT    (1)
#define TYPE_REQUEST    (2)
#define TYPE_MISSING    (3)
#define TYPE_NORMAL     (4)

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

static void queue_push(struct message_queue* mq, struct message* m) {
  if (mq->head == nullptr) {
    mq->head = mq->tail = m;
  }
  else {
    mq->tail->next = m;
    mq->tail = m;
  }
}

static struct message* queue_pop(struct message_queue* mq, int id) {
  if (mq->head == nullptr)
    return nullptr;

  struct message* m = mq->head;
  if (m->id != id)
    return nullptr;

  mq->head = m->next;
  m->next = nullptr;
  if (mq->head == nullptr)
    mq->tail = nullptr;
  return m;
}

struct array {
  int cap;
  int n;
  int* a;
};

static void array_insert(struct array* a, int id) {
  int i{};
  for (; i < a->n; ++i) {
    if (a->a[i] == id)
      return;
    if (a->a[i] > id)
      break;
  }
  // insert id before index [i]
  if (a->n >= a->cap) {
    if (a->cap == 0)
      a->cap = 16;
    else
      a->cap *= 2;
    a->a = (int*)realloc(a->a, sizeof(int) * a->cap);
  }
  for (int j = a->n; j > i; --j)
    a->a[j] = a->a[j - 1];
  a->a[i] = id;
  ++a->n;
}

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

static int gen_id(struct rudp* u, const uint8_t* buffer) {
  int id = buffer[0] * 256 + buffer[1];
  id |= u->recv_id_max & ~0xffff;
  if (id < u->recv_id_max - 0x8000)
    id += 0x10000;
  else if (id > u->recv_id_max + 0x8000)
    id -= 0x10000;
  return id;
}

static void clear_outpackage(struct rudp* u) {
  struct rudp_package* package = u->send_package;
  while (package) {
    struct rudp_package* next_package = package->next;
    free(package);
    package = next_package;
  }
  u->send_package = nullptr;
}

static void free_message_list(struct message* m) {
  while (m) {
    struct message* next_message = m->next;
    free(m);
    m = next_message;
  }
}

// region: message methods
static struct message* new_message(struct rudp* u, const uint8_t* buffer, int sz) {
  struct message* m = u->free_list;
  if (m) {
    u->free_list = m->next;
    if (m->cap < sz) {
      free(m);
      m = nullptr;
    }
  }
  if (m == nullptr) {
    int cap = sz;
    if (cap < GENERAL_PACKAGE)
      cap = GENERAL_PACKAGE;
    m = (struct message*)malloc(sizeof(*m) + cap);
    if (m)
      m->cap = cap;
  }
  if (m == nullptr)
    return m;

  m->sz = sz;
  m->buffer = (uint8_t*)(m + 1);
  if (sz > 0 && buffer)
    memcpy(m->buffer, buffer, sz);
  m->tick = 0;
  m->id = 0;
  m->next = nullptr;
  return m;
}

static void delete_message(struct rudp* u, struct message* m) {
  m->next = u->free_list;
  u->free_list = m;
}

static void insert_message(struct rudp* u, int id, const uint8_t* buffer, int sz) {
  if (id < u->recv_id_min)
    return;
  if (id > u->recv_id_max || u->recv_queue.head == nullptr) {
    struct message* m = new_message(u, buffer, sz);
    m->id = id;
    queue_push(&u->recv_queue, m);
    u->recv_id_max = id;
  }
  else {
    struct message* m = u->recv_queue.head;
    struct message** last = &u->recv_queue.head;
    do {
      if (m->id == id)
        return;
      if (m->id > id) {
        struct message* temp_message = new_message(u, buffer, sz);
        temp_message->id = id;
        temp_message->next = m;
        *last = temp_message;
        return;
      }
      last = &m->next;
      m = m->next;
    } while (m);
  }
}

static void add_missing(struct rudp* u, int id) {
  insert_message(u, id, nullptr, -1);
}
// endregion: message methods

static void clear_send_expired(struct rudp* u, int tick) {
  struct message* m = u->send_history.head;
  struct message* last = nullptr;
  while (m) {
    if (m->tick >= tick)
      break;
    last = m;
    m = m->next;
  }
  if (last) {
    last->next = u->free_list;
    u->free_list = u->send_history.head;
  }
  u->send_history.head = m;
  if (m == nullptr)
    u->send_history.tail = nullptr;
}

static void add_request(struct rudp* u, int id) {
  array_insert(&u->send_argain, id);
}

// region: package functions
// endregion: package functions

// region: rudp export methods
struct rudp* rudp_new(int send_delay, int expired_time) {
  struct rudp* u = (struct rudp*)malloc(sizeof(*u));
  if (u) {
    memset(u, 0, sizeof(*u));
    u->send_delay = send_delay;
    u->expired = expired_time;
  }

  return u;
}

void rudp_delete(struct rudp* u) {
  free_message_list(u->send_queue.head);
  free_message_list(u->recv_queue.head);
  free_message_list(u->send_history.head);
  free_message_list(u->free_list);
  clear_outpackage(u);
  free(u->send_argain.a);
  free(u);
}

void rudp_send(struct rudp* u, const char* buffer, int sz) {
  assert(sz <= MAX_PACKAGE);
  struct message* m = new_message(u, (const uint8_t*)buffer, sz);
  m->id = u->send_id++;
  m->tick = u->current_tick;
  queue_push(&u->send_queue, m);
}

int rudp_recv(struct rudp* u, char buffer[MAX_PACKAGE]) {
  if (u->corrupt) {
    u->corrupt = 0;
    return -1;
  }
  struct message* m = queue_pop(&u->recv_queue, u->recv_id_min);
  if (m == nullptr)
    return 0;
  ++u->recv_id_min;
  int sz = m->sz;
  if (sz > 0)
    memcpy(buffer, m->buffer, sz);
  delete_message(u, m);

  return sz;
}
// endregion: rudp export methods

}
