/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
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
#ifndef __LAYER_HEADER_H__
#define __LAYER_HEADER_H__ 

#define MAX_LAYERS    (256)

#include "memory.h"
#include "multitask.h"

struct layer_mgr_t;
typedef struct layer_t layer_t;
struct layer_t {
  unsigned char* buf;   /* layer buffer */
  int w_size, h_size;   /* w_size * h_size */
  int x, y;             /* position */
  int alpha;            /* transparent color */
  int height;           /* layer height */
  int flags;
  struct layer_mgr_t* mgr;  /* object of layer manager */
  task_t* task;
};

typedef struct layer_mgr_t {
  unsigned char* vram;
  unsigned char* map;   /* same size as layer, as layer's map */
  int w_size, h_size;   /* picture size (w_size * h_size) */
  int top;              /* top height of the layer */
  layer_t* layers_addr[MAX_LAYERS]; /* layers' address by ASC */
  layer_t  layers[MAX_LAYERS];      /* layers array */
} layer_mgr_t;

extern layer_mgr_t* layer_mgr_init(mem_mgr_t* mem_mgr, 
    unsigned char* vram, int w_size, int h_size);
extern layer_t* layer_alloc(layer_mgr_t* layer_mgr);
extern void layer_free(layer_t* layer);
extern void layer_setbuf(layer_t* layer, unsigned char* buf, 
    int w_size, int h_size, int alpha);
extern void layer_updown(layer_t* layer, int height);
extern void layers_refresh(layer_t* layer, int x0, int y0, int x1, int y1);
extern void layer_slide(layer_t* layer, int x, int y);

#endif  /* __LAYER_HEADER_H__ */
