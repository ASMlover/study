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
static void extrat_package(struct rudp* u, const uint8_t* buffer, int sz) {
  while (sz > 0) {
    int len = buffer[0];
    if (len > 127) {
      if (sz <= 1) {
        u->corrupt = 1;
        return;
      }
      len = (len * 256 + buffer[1]) & 0x7fff;
      buffer += 2;
      sz -= 2;
    }
    else {
      buffer += 1;
      sz -= 1;
    }

    switch (len) {
    case TYPE_IGNORE:
      if (u->send_argain.n == 0)
        array_insert(&u->send_argain, u->recv_id_min); // request next package id
      break;
    case TYPE_CORRUPT:
      u->corrupt = 1;
      return;
    case TYPE_REQUEST:
    case TYPE_MISSING:
      if (sz < 2) {
        u->corrupt = 1;
        return;
      }
      if (len == TYPE_REQUEST)
        add_request(u, gen_id(u, buffer));
      else
        add_missing(u, gen_id(u, buffer));
      buffer += 2;
      sz -= 2;
      break;
    default:
      len -= TYPE_NORMAL;
      if (sz < len + 2) {
        u->corrupt = 1;
        return;
      }
      else {
        int id = gen_id(u, buffer);
        insert_message(u, id, buffer + 2, len);
      }
      buffer += len + 2;
      sz -= len + 2;
      break;
    }
  }
}

struct temp_buffer {
  uint8_t buf[GENERAL_PACKAGE];
  int sz;
  struct rudp_package* head;
  struct rudp_package* tail;
};

static void new_package(struct rudp* u, struct temp_buffer* tmp) {
  struct rudp_package* p = (struct rudp_package*)malloc(sizeof(*p) + tmp->sz);
  p->next = nullptr;
  p->buffer = (char*)(p + 1);
  p->sz = tmp->sz;
  memcpy(p->buffer, tmp->buf, tmp->sz);
  if (tmp->tail == nullptr) {
    tmp->head = tmp->tail = p;
  }
  else {
    tmp->tail->next = p;
    tmp->tail = p;
  }
  tmp->sz = 0;
}

static int fill_header(uint8_t* buf, int len, int id) {
  int sz;
  if (len < 128) {
    buf[0] = len;
    ++buf;
    sz = 1;
  }
  else {
    buf[0] = ((len & 0x7f00) >> 8) | 0x80;
    buf[1] = len & 0xff;
    buf += 2;
    sz = 2;
  }
  buf[0] = (id & 0xff00) >> 8;
  buf[1] = id & 0xff;
  return sz + 2;
}

static void pack_request(struct rudp* u, struct temp_buffer* temp, int id, int tag) {
  int sz = GENERAL_PACKAGE - temp->sz;
  if (sz < 3)
    new_package(u, temp);
  uint8_t* buffer = temp->buf + temp->sz;
  temp->sz += fill_header(buffer, tag, id);
}

static void pack_message(struct rudp* u, struct temp_buffer* temp, struct message* m) {
  int sz = GENERAL_PACKAGE - temp->sz;
  if (m->sz > GENERAL_PACKAGE - 4) {
    if (temp->sz > 0)
      new_package(u, temp);
    sz = 4 + m->sz;
    struct rudp_package* p = (struct rudp_package*)malloc(sizeof(*p) + sz);
    p->next = nullptr;
    p->buffer = (char*)(p + 1);
    p->sz = sz;
    fill_header((uint8_t*)p->buffer, m->sz + TYPE_NORMAL, m->id);
    memcpy(p->buffer + 4, m->buffer, m->sz);
    if (temp->tail == nullptr) {
      temp->head = temp->tail = p;
    }
    else {
      temp->tail->next = p;
      temp->tail = p;
    }
    return;
  }
  if (sz < 4 + m->sz)
    new_package(u, temp);
  uint8_t* buf = temp->buf + temp->sz;
  int len = fill_header(buf, m->sz + TYPE_NORMAL, m->id);
  temp->sz += len + m->sz;
  buf += len;
  memcpy(buf, m->buffer, m->sz);
}

static void request_missing(struct rudp* u, struct temp_buffer* temp) {
  int id = u->recv_id_min;
  struct message* m = u->recv_queue.head;
  while (m) {
    if (m->id > id) {
      for (int i = id; i < m->id; ++i)
        pack_request(u, temp, i, TYPE_REQUEST);
    }
    id = m->id + 1;
    m = m->next;
  }
}

static void reply_request(struct rudp* u, struct temp_buffer* temp) {
  struct message* history = u->send_history.head;
  for (int i = 0; i< u->send_argain.n; ++i) {
    int id = u->send_argain.a[i];
    if (id < u->recv_id_min)
      continue; // already received, ignore

    for (;;) {
      if (history == nullptr || id < history->id) {
        // expired
        pack_request(u, temp, id, TYPE_MISSING);
        break;
      }
      else if (id == history->id) {
        pack_message(u, temp, history);
        break;
      }
      history = history->next;
    }
  }
  u->send_argain.n = 0;
}

static void send_message(struct rudp* u, struct temp_buffer* temp) {
  struct message* m = u->send_queue.head;
  while (m) {
    pack_message(u, temp, m);
    m = m->next;
  }
  if (u->send_queue.head) {
    if (u->send_history.tail == nullptr) {
      u->send_history = u->send_queue;
    }
    else {
      u->send_history.tail->next = u->send_queue.head;
      u->send_history.tail = u->send_queue.tail;
    }
    u->send_queue.head = nullptr;
    u->send_queue.tail = nullptr;
  }
}

static struct rudp_package* gen_outpackage(struct rudp* u) {
  // [1] request missing
  // [2] reply request
  // [3] send message
  // [4] send heartbeat

  struct temp_buffer temp{};

  request_missing(u, &temp);
  reply_request(u, &temp);
  send_message(u, &temp);

  if (temp.head == nullptr) {
    if (temp.sz == 0) {
      temp.buf[0] = TYPE_IGNORE;
      temp.sz = 1;
    }
  }
  new_package(u, &temp);
  return temp.head;
}
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

struct rudp_package* rudp_update(struct rudp* u, const void* buffer, int sz, int tick) {
  u->current_tick += tick;
  clear_outpackage(u);
  extrat_package(u, (const uint8_t*)buffer, sz);
  if (u->current_tick >= u->last_expired_tick + u->expired) {
    clear_send_expired(u, u->last_expired_tick);
    u->last_expired_tick = u->current_tick;
  }
  if (u->current_tick >= u->last_send_tick + u->send_delay) {
    u->send_package = gen_outpackage(u);
    u->last_send_tick = u->current_tick;
    return u->send_package;
  }
  else {
    return nullptr;
  }
}
// endregion: rudp export methods

}
