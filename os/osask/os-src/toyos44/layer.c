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
#include "memory.h"
#include "layer.h"


#define LAYER_ACTIVE    (1)


layer_mgr_t* 
layer_mgr_init(mem_mgr_t* mem_mgr, 
    unsigned char* vram, int w_size, int h_size)
{
  layer_mgr_t* mgr;
  int i;

  mgr = (layer_mgr_t*)mem_mgr_alloc_4k(mem_mgr, sizeof(layer_mgr_t));
  if (0 == mgr)
    goto __error;

  mgr->vram   = vram;
  mgr->w_size = w_size;
  mgr->h_size = h_size;
  mgr->top    = -1;     /* there is no layer */
  for (i = 0; i < MAX_LAYERS; ++i) 
    mgr->layers[i].flags = 0;

__error:
  return mgr;
}

layer_t* 
layer_alloc(layer_mgr_t* layer_mgr)
{
  layer_t* layer;
  int i;

  for (i = 0; i < MAX_LAYERS; ++i) {
    if (0 == layer_mgr->layers[i].flags) {
      layer = &layer_mgr->layers[i];
      layer->flags = LAYER_ACTIVE;  /* mark that the layer is active */
      layer->height = -1;           /* hide */
      return layer;
    }
  }

  return 0; /* all layers are activity */
}

void 
layer_free(layer_mgr_t* layer_mgr, layer_t* layer)
{
  if (layer->height >= 0)
    layer_updown(layer_mgr, layer, -1);
  layer->flags = 0;
}

void 
layer_setbuf(layer_t* layer, unsigned char* buf, 
    int w_size, int h_size, int alpha)
{
  layer->buf    = buf;
  layer->w_size = w_size;
  layer->h_size = h_size;
  layer->alpha  = alpha;
}

void 
layer_updown(layer_mgr_t* layer_mgr, layer_t* layer, int height)
{
  int h, old_height = layer->height;

  /* correct the wrong height */
  if (height > layer_mgr->top + 1)
    height = layer_mgr->top + 1;
  if (height < -1)
    height = -1;
  layer->height = height;   /* setting the height */

  /* rearrangement the layers_addr[] */
  if (old_height > height) {
    if (height >= 0) {
      for (h = old_height; h > height; --h) {
        layer_mgr->layers_addr[h] = layer_mgr->layers_addr[h - 1];
        layer_mgr->layers_addr[h]->height = h;
      }
      layer_mgr->layers_addr[height] = layer;
    }
    else {
      if (layer_mgr->top > old_height) {
        for (h = old_height; h < layer_mgr->top; ++h) {
          layer_mgr->layers_addr[h] = layer_mgr->layers_addr[h + 1];
          layer_mgr->layers_addr[h]->height = h;
        }
      }
      --layer_mgr->top;
    }

    layers_refresh(layer_mgr);
  }
  else if (old_height < height) {
    if (old_height >= 0) {
      for (h = old_height; h < height; ++h) {
        layer_mgr->layers_addr[h] = layer_mgr->layers_addr[h + 1];
        layer_mgr->layers_addr[h]->height = h;
      }
      layer_mgr->layers_addr[height] = layer;
    }
    else {
      for (h = layer_mgr->top; h >= height; --h) {
        layer_mgr->layers_addr[h + 1] = layer_mgr->layers_addr[h];
        layer_mgr->layers_addr[h + 1]->height = h + 1;
      }
      layer_mgr->layers_addr[height] = layer;
      ++layer_mgr->top;
    }

    layers_refresh(layer_mgr);
  }
}

void 
layers_refresh(layer_mgr_t* layer_mgr)
{
  int h, width, height, x, y;
  unsigned char* buf;
  unsigned char  c;
  unsigned char* vram = layer_mgr->vram;
  layer_t* layer;

  for (h = 0; h <= layer_mgr->top; ++h) {
    layer = layer_mgr->layers_addr[h];
    buf = layer->buf;

    for (height = 0; height < layer->h_size; ++height) {
      y = layer->y + height;
      for (width = 0; width < layer->w_size; ++width) {
        x = layer->x + width;
        c = buf[height * layer->w_size + width];
        if (c != layer->alpha)
          vram[y * layer_mgr->w_size + x] = c;
      }
    }
  }
}

void 
layer_slide(layer_mgr_t* layer_mgr, layer_t* layer, int x, int y)
{
  layer->x = x;
  layer->y = y;
  if (layer->height >= 0)
    layers_refresh(layer_mgr);
}
