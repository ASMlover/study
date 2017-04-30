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
#include "njlog.h"
#include "njmem.h"
#include "njset.h"
#include "njvm.h"

#define Nj_ASGC(ob)   ((GCHead*)(ob) - 1)
#define Nj_REFCNT(ob) (Nj_ASGC(ob)->refcnt)
#define Nj_NEWREF(ob) (Nj_REFCNT(ob) = 1)
#define Nj_COLOUR(ob) (Nj_ASGC(ob)->color)
#define Nj_NEWCOL(ob) (Nj_COLOUR(ob) = BLACK)

typedef enum _color {
  BLACK,
  PURPLE,
  GREY,
  WHITE,
} NjColor;

typedef struct _gc {
  Nj_ssize_t refcnt;
  Nj_uchar_t color;
} GCHead;

typedef struct _vm {
  NjObject_VM_HEAD;

  Nj_int_t objcnt;
  Nj_int_t maxobj;
} NjVMObject;

static NjSet* candidates;

static void _njrecycler_release(NjObject*);

static void
_njrecycler_reclaim(NjObject* vm, NjObject* obj) {
  njlog_debug("collecting NjObject<%p, `%s`> ...\n",
      obj, obj->ob_type->tp_name);
  njord_freeobj(obj, sizeof(GCHead));
  --Nj_VM(vm)->objcnt;
}

static void
_njrecycler_candidate(NjObject* obj) {
  if (Nj_COLOUR(obj) != PURPLE) {
    Nj_COLOUR(obj) = PURPLE;
    njset_add(candidates, obj);
  }
}

static void
_njrecycler_addref(NjObject* obj) {
  if (obj != NULL) {
    ++Nj_REFCNT(obj);
    Nj_COLOUR(obj) = BLACK;
  }
}

static void
_njrecycler_delref(NjObject* obj)  {
  if (obj != NULL) {
    if (--Nj_REFCNT(obj) == 0)
      _njrecycler_release(obj);
    else
      _njrecycler_candidate(obj);
  }
}

static void
_njrecycler_release(NjObject* obj) {
  if (Nj_ISPAIR(obj)) {
    _njrecycler_delref(njord_pairgetter(obj, "head"));
    _njrecycler_delref(njord_pairgetter(obj, "tail"));
  }
  Nj_COLOUR(obj) = BLACK;

  if (!njset_contains(candidates, obj))
    _njrecycler_reclaim(njord_startup_gc(), obj);
}

static void
_njrecycler_vm_destroy(NjObject* vm) {
  Nj_UNUSED(vm);
  njset_dealloc(candidates);
}

static void
njrecycler_dealloc(NjObject* vm) {
  njvm_freevm(vm, _njrecycler_vm_destroy);
}

static NjIntObject*
_njrecycler_newint(NjObject* vm, Nj_int_t value) {
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, NULL, NULL);
  Nj_NEWREF(obj);
  Nj_NEWCOL(obj);
  ++Nj_VM(vm)->objcnt;
  return obj;
}

static NjObject*
njrecycler_pushint(NjObject* vm, Nj_int_t value) {
  return njvm_pushint(vm, value,
      Nj_VM(vm)->objcnt >= Nj_VM(vm)->maxobj, _njrecycler_newint);
}

static NjPairObject*
_njrecycler_newpair(NjObject* vm) {
  NjPairObject* obj = (NjPairObject*)njord_newpair(sizeof(GCHead), NULL, NULL);
  Nj_NEWREF(obj);
  Nj_NEWCOL(obj);
  ++Nj_VM(vm)->objcnt;
  return obj;
}

static NjObject*
njrecycler_pushpair(NjObject* vm) {
  return njvm_pushpair(vm,
      Nj_VM(vm)->objcnt >= Nj_VM(vm)->maxobj, _njrecycler_newpair, NULL);
}

static void
njrecycler_setpair(NjObject* obj, NjObject* head, NjObject* tail) {
  if (head != NULL) {
    _njrecycler_addref(head);
    _njrecycler_delref(njord_pairgetter(obj, "head"));
    njord_pairsetter(obj, "head", head);
  }

  if (tail != NULL) {
    _njrecycler_addref(tail);
    _njrecycler_delref(njord_pairgetter(obj, "tail"));
    njord_pairsetter(obj, "tail", tail);
  }
}

static void
njrecycler_pop(NjObject* vm) {
  _njrecycler_delref(njvm_pop(vm));
}

static void
_njrecycler_mark_grey(NjObject* obj) {
#define Nj_MARKGREY(ob) do {\
  if ((ob) != NULL) {\
    --Nj_REFCNT(ob);\
    _njrecycler_mark_grey((ob));\
  }\
} while (0)

  if (Nj_COLOUR(obj) != GREY) {
    Nj_COLOUR(obj) = GREY;
    if (Nj_ISPAIR(obj)) {
      Nj_MARKGREY(njord_pairgetter(obj, "head"));
      Nj_MARKGREY(njord_pairgetter(obj, "tail"));
    }
  }

#undef Nj_MARKGREY
}

static void
_njrecycler_mark_candidate(NjObject* obj, void* arg) {
  if (Nj_COLOUR(obj) == PURPLE) {
    _njrecycler_mark_grey(obj);
  }
  else {
    njset_remove(candidates, obj);
    if (Nj_COLOUR(obj) == BLACK && Nj_REFCNT(obj) == 0)
      _njrecycler_reclaim((NjObject*)arg, obj);
  }
}

static void
_njrecycler_mark_candidates(NjObject* vm) {
  njset_traverse(candidates, _njrecycler_mark_candidate, vm);
}

static void
_njrecycler_scan_black(NjObject* obj) {
#define Nj_SCANBLACK(ob) do {\
  if ((ob) != NULL) {\
    ++Nj_REFCNT(ob);\
    if (Nj_COLOUR(ob) != BLACK)\
      _njrecycler_scan_black((ob));\
  }\
} while (0)

  Nj_COLOUR(obj) = BLACK;
  if (Nj_ISPAIR(obj)) {
    Nj_SCANBLACK(njord_pairgetter(obj, "head"));
    Nj_SCANBLACK(njord_pairgetter(obj, "tail"));
  }

#undef Nj_SCANBLACK
}

static void
_njrecycler_scan(NjObject* obj, void* arg) {
#define Nj_SCAN(ob) { if ((ob) != NULL) _njrecycler_scan((ob), arg);}

  if (Nj_COLOUR(obj) == GREY) {
    if (Nj_REFCNT(obj) > 0) {
      _njrecycler_scan_black(obj);
    }
    else {
      Nj_COLOUR(obj) = WHITE;
      if (Nj_ISPAIR(obj)) {
        Nj_SCAN(njord_pairgetter(obj, "head"));
        Nj_SCAN(njord_pairgetter(obj, "tail"));
      }
    }
  }

#undef Nj_SCAN
}

static void
_njrecycler_collect_white(NjObject* obj, void* arg) {
#define Nj_COLLECT_WHITE(ob) do {\
  if ((ob) != NULL) \
    _njrecycler_collect_white((ob), arg);\
} while (0)

  njset_remove(candidates, obj);
  if (Nj_COLOUR(obj) == WHITE && !njset_contains(candidates, obj)) {
    Nj_COLOUR(obj) = BLACK;
    if (Nj_ISPAIR(obj)) {
      Nj_COLLECT_WHITE(njord_pairgetter(obj, "head"));
      Nj_COLLECT_WHITE(njord_pairgetter(obj, "tail"));
    }
    _njrecycler_reclaim((NjObject*)arg, obj);
  }

#undef Nj_COLLECT_WHITE
}

static void
njrecycler_collect(NjObject* vm) {
  Nj_int_t old_objcnt = Nj_VM(vm)->objcnt;

  _njrecycler_mark_candidates(vm); /* mark all candidates objects to GREY */
  njset_traverse(candidates, _njrecycler_scan, NULL); /* colour to WHITE */
  njset_traverse(candidates, _njrecycler_collect_white, vm); /* recycle */

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
  njrecycler_dealloc, /* gc_dealloc */
  njrecycler_pushint, /* gc_pushint */
  njrecycler_pushpair, /* gc_pushpair */
  njrecycler_setpair, /* gc_setpair */
  njrecycler_pop, /* gc_pop */
  njrecycler_collect, /* gc_collect */
};

static NjTypeObject NjRecycler_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "recyclerrefs_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_debug */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njrecycler_vm_init(NjObject* vm) {
  Nj_VM(vm)->ob_type = &NjRecycler_Type;
  Nj_VM(vm)->objcnt = 0;
  Nj_VM(vm)->maxobj = Nj_GC_INITTHRESHOLD;
  candidates = njset_create();
}

NjObject*
njrefs4_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njrecycler_vm_init);
}
