/*
 * Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <string.h>
#include "njlog.h"
#include "njmem.h"
#include "njset.h"
#include "njvm.h"

/* TODO: there's some bugs of this gc, need fix it */

#define Nj_ASGC(ob)       ((GCHead*)(ob) - 1)
#define Nj_REFCNT(ob)     (Nj_ASGC(ob)->refcnt)
#define Nj_DIRTY(ob)      (Nj_ASGC(ob)->dirty)
#define Nj_NEWREF(ob)     (Nj_REFCNT(ob) = 0)
#define Nj_CLEAN          (-1)
#define Nj_INITDIRTY(ob)  (Nj_ASGC(ob)->dirty = Nj_CLEAN)
#define Nj_ISDIRTY(ob)    (Nj_ASGC(ob)->dirty != Nj_CLEAN)


typedef struct _gc {
  Nj_ssize_t refcnt;
  Nj_int_t dirty;
} GCHead;

typedef struct _vm {
  NjObject_VM_HEAD;

  Nj_int_t objcnt;
  Nj_int_t maxobj;
} NjVMObject;

typedef struct _logentry {
  NjObject* log[Nj_VMSTACK];
  Nj_int_t count;
} LogEntry;

static NjSet* zct; /* zero counting table */
static LogEntry logentry;

static void
_njcoalesced_reclaim(NjObject* vm, NjObject* obj) {
  njlog_debug("collecting NjObject<%p, `%s`> ...\n",
      obj, obj->ob_type->tp_name);
  njord_freeobj(obj, sizeof(GCHead));
  --Nj_VM(vm)->objcnt;
}

static void
_njcoalesced_logentry_init(void) {
  memset(&logentry, 0, sizeof(logentry));
}

static void
_njcoalesced_logentry_append(NjObject* obj) {
  logentry.log[logentry.count++] = obj;
}

static Nj_int_t
_njcoalesced_logentry_append_commit(NjObject* obj) {
  Nj_int_t i = logentry.count;
  logentry.log[logentry.count++] = obj;
  return i;
}

static void
_njcoalesced_delref(NjObject* vm, NjObject* obj) {
  if (obj != NULL && --Nj_REFCNT(obj) == 0) {
    if (Nj_ISPAIR(obj)) {
      _njcoalesced_delref(vm, njord_pairgetter(obj, "head"));
      _njcoalesced_delref(vm, njord_pairgetter(obj, "tail"));
    }
    _njcoalesced_reclaim(vm, obj);
  }
}

static Nj_bool_t
_njcoalesced_isdirty(NjObject* obj) {
  return Nj_ISDIRTY(obj);
}

static void
_njcoalesced_setdirty(NjObject* obj, Nj_int_t solt) {
  Nj_ASGC(obj)->dirty = solt;
}

static void
_njcoalesced_log(NjObject* obj) {
#define Nj_LOGAPPEND(ob) {\
  if ((ob) != NULL)\
    _njcoalesced_logentry_append(ob);\
}

  if (obj == NULL)
    return;
  if (Nj_ISPAIR(obj)) {
    Nj_LOGAPPEND(njord_pairgetter(obj, "head"));
    Nj_LOGAPPEND(njord_pairgetter(obj, "tail"));
  }
  if (!_njcoalesced_isdirty(obj)) {
    Nj_int_t solt = _njcoalesced_logentry_append_commit(obj);
    _njcoalesced_setdirty(obj, solt);
  }

#undef Nj_LOGAPPEND
}

static void
_njcoalesced_incnew(NjObject* obj) {
#define Nj_INCOBJ(ob) {if ((ob) != NULL) ++Nj_REFCNT(ob);}

  if (Nj_ISPAIR(obj)) {
    Nj_INCOBJ(njord_pairgetter(obj, "head"));
    Nj_INCOBJ(njord_pairgetter(obj, "tail"));
  }

#undef Nj_INCOBJ
}

static void
_njcoalesced_decold(Nj_int_t entry) {
#define Nj_DECOBJ(ob) {\
  if ((ob) != NULL && --Nj_REFCNT(ob) == 0) njset_add(zct, (ob));\
}

  NjObject* obj = logentry.log[entry];
  if (obj != NULL && Nj_REFCNT(obj) > 0 && Nj_ISPAIR(obj)) {
    Nj_DECOBJ(njord_pairgetter(obj, "head"));
    Nj_DECOBJ(njord_pairgetter(obj, "tail"));
  }

#undef Nj_DECOBJ
}

static void
_njcoalesced_process_logentry(void) {
  for (Nj_int_t i = 0; i < logentry.count; ++i) {
    NjObject* obj = logentry.log[i];
    if (_njcoalesced_isdirty(obj)) {
      Nj_INITDIRTY(obj);
      _njcoalesced_incnew(obj);
      _njcoalesced_decold(i);
    }
  }
  _njcoalesced_logentry_init();
}

static void
_njcoalesced_vm_destroy(NjObject* vm) {
  Nj_UNUSED(vm);
  njset_dealloc(zct);
}

static void
njcoalesced_dealloc(NjObject* vm) {
  njvm_freevm(vm, _njcoalesced_vm_destroy);
}

static NjIntObject*
_njcoalesced_newint(NjObject* vm, Nj_int_t value) {
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, NULL, NULL);
  Nj_NEWREF(obj);
  Nj_INITDIRTY(obj);
  ++Nj_VM(vm)->objcnt;
  njset_add(zct, Nj_ASOBJ(obj));
  return obj;
}

static NjObject*
njcoalesced_pushint(NjObject* vm, Nj_int_t value) {
  return njvm_pushint(vm, value,
      Nj_VM(vm)->objcnt >= Nj_VM(vm)->maxobj, _njcoalesced_newint);
}

static NjPairObject*
_njcoalesced_newpair(NjObject* vm) {
  NjPairObject* obj = (NjPairObject*)njord_newpair(sizeof(GCHead), NULL, NULL);
  Nj_NEWREF(obj);
  Nj_INITDIRTY(obj);
  ++Nj_VM(vm)->objcnt;
  njset_add(zct, Nj_ASOBJ(obj));
  return obj;
}

static NjObject*
njcoalesced_pushpair(NjObject* vm) {
  NjObject* obj = njvm_pushpair(vm,
      Nj_VM(vm)->objcnt >= Nj_VM(vm)->maxobj, _njcoalesced_newpair, NULL);

  if (!_njcoalesced_isdirty(obj))
    _njcoalesced_log(obj);
  return obj;
}

static void
njcoalesced_setpair(NjObject* obj, NjObject* head, NjObject* tail) {
  if (!_njcoalesced_isdirty(obj))
    _njcoalesced_log(obj);

  if (head != NULL)
    njord_pairsetter(obj, "head", head);
  if (tail != NULL)
    njord_pairsetter(obj, "tail", tail);
}

static void
_njcoalesced_sweepzct_visit(NjObject* obj, void* arg) {
  NjObject* vm = (NjObject*)arg;
  njset_remove(zct, obj);
  if (Nj_REFCNT(obj) == 0) {
    if (Nj_ISPAIR(obj)) {
      _njcoalesced_delref(vm, njord_pairgetter(obj, "head"));
      _njcoalesced_delref(vm, njord_pairgetter(obj, "tail"));
    }
    _njcoalesced_reclaim(vm, obj);
  }
}

static void
njcoalesced_collect(NjObject* vm) {
  Nj_int_t old_objcnt = Nj_VM(vm)->objcnt;

  _njcoalesced_process_logentry();
  njset_traverse(zct, _njcoalesced_sweepzct_visit, vm);

  if (Nj_VM(vm)->maxobj < Nj_GC_MAXTHRESHOLD) {
    Nj_VM(vm)->maxobj = Nj_VM(vm)->objcnt << 1;
    if (Nj_VM(vm)->maxobj > Nj_GC_MAXTHRESHOLD)
      Nj_VM(vm)->maxobj = Nj_GC_MAXTHRESHOLD;
  }
  njlog_info("<%s> collected [%d] objects, [%d] remaining.\n",
      Nj_VM(vm)->ob_type->tp_name,
      old_objcnt - Nj_VM(vm)->objcnt, Nj_VM(vm)->objcnt);
}

static NjGCMethods gc_methods = {
  njcoalesced_dealloc, /* gc_dealloc */
  njcoalesced_pushint, /* gc_pushint */
  njcoalesced_pushpair, /* gc_pushpair */
  njcoalesced_setpair, /* gc_setpair */
  0, /* gc_pop */
  njcoalesced_collect, /* gc_collect */
};

static NjTypeObject NjCoalesced_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "coalescedrefs_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_debug */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njcoalesced_vm_init(NjObject* vm) {
  Nj_VM(vm)->ob_type = &NjCoalesced_Type;
  Nj_VM(vm)->objcnt = 0;
  Nj_VM(vm)->maxobj = Nj_GC_MAXTHRESHOLD;
  zct = njset_create();
  _njcoalesced_logentry_init();
}

NjObject*
njrefs5_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njcoalesced_vm_init);
}
