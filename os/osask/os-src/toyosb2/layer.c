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



static void 
layer_refresh_map(layer_mgr_t* layer_mgr, 
    int x0, int y0, int x1, int y1, int h0)
{
  int h, width, height, x, y;
  int left, top, right, bottom;
  unsigned char* buf;
  unsigned char  sid;   /* layer's ID */
  unsigned char* map = layer_mgr->map;
  layer_t* layer;

  if (x0 < 0) 
    x0 = 0;
  if (y0 < 0)
    y0 = 0;
  if (x1 > layer_mgr->w_size)
    x1 = layer_mgr->w_size;
  if (y1 > layer_mgr->h_size)
    y1 = layer_mgr->h_size;

  for (h = h0; h <= layer_mgr->top; ++h) {
    layer = layer_mgr->layers_addr[h];
    sid   = layer - layer_mgr->layers;
    buf   = layer->buf;

    left   = x0 - layer->x;
    top    = y0 - layer->y;
    right  = x1 - layer->x;
    bottom = y1 - layer->y;
    if (left < 0)
      left = 0;
    if (top < 0)
      top = 0;
    if (right > layer->w_size)
      right = layer->w_size;
    if (bottom > layer->h_size)
      bottom = layer->h_size;
    for (height = top; height < bottom; ++height) {
      y = layer->y + height;
      for (width = left; width < right; ++width) {
        x = layer->x + width;
        if (buf[height * layer->w_size + width] != layer->alpha)
          map[y * layer_mgr->w_size + x] = sid;
      }
    }
  }
}
  
static  void 
layers_refresh_sub(layer_mgr_t* layer_mgr, 
    int x0, int y0, int x1, int y1, int h0, int h1)
{
  int h, width, height, x, y;
  int left, top, right, bottom;
  unsigned char* buf;
  unsigned char  c;
  unsigned char* vram = layer_mgr->vram;
  unsigned char* map = layer_mgr->map;
  unsigned char  sid;
  layer_t* layer;

  /* donot refresh out of the screen */
  if (x0 < 0)
    x0 = 0;
  if (y0 < 0)
    y0 = 0;
  if (x1 > layer_mgr->w_size)
    x1 = layer_mgr->w_size;
  if (y1 > layer_mgr->h_size)
    y1 = layer_mgr->h_size;

  /* refresh h0 layer and layers above h0 */
  for (h = h0; h <= h1; ++h) {
    layer = layer_mgr->layers_addr[h];
    buf = layer->buf;
    sid = layer - layer_mgr->layers;

    left    = x0 - layer->x;
    top     = y0 - layer->y;
    right   = x1 - layer->x;
    bottom  = y1 - layer->y;
    if (left < 0)
      left = 0;
    if (top < 0)
      top = 0;
    if (right > layer->w_size)
      right = layer->w_size;
    if (bottom > layer->h_size)
      bottom = layer->h_size;

    for (height = top; height < bottom; ++height) {
      y = layer->y + height;
      for (width = left; width < right; ++width) {
        x = layer->x + width;
        c = buf[height * layer->w_size + width];
        if (map[y * layer_mgr->w_size + x] == sid)
          vram[y * layer_mgr->w_size + x] = c;
      }
    }
  }
}


layer_mgr_t* 
layer_mgr_init(mem_mgr_t* mem_mgr, 
    unsigned char* vram, int w_size, int h_size)
{
  layer_mgr_t* mgr;
  int i;

  mgr = (layer_mgr_t*)mem_mgr_alloc_4k(mem_mgr, sizeof(layer_mgr_t));
  if (0 == mgr)
    goto __error;
  mgr->map = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, w_size * h_size);
  if (0 == mgr->map) {
    mem_mgr_free_4k(mem_mgr, (int)mgr, sizeof(layer_mgr_t));
    goto __error;
  }

  mgr->vram   = vram;
  mgr->w_size = w_size;
  mgr->h_size = h_size;
  mgr->top    = -1;     /* there is no layer */
  for (i = 0; i < MAX_LAYERS; ++i) {
    mgr->layers[i].flags = 0;
    mgr->layers[i].mgr   = mgr;
  }

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
layer_free(layer_t* layer)
{
  if (layer->height >= 0)
    layer_updown(layer, -1);
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
layer_updown(layer_t* layer, int height)
{
  layer_mgr_t* layer_mgr = layer->mgr;
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
      layer_refresh_map(layer_mgr, layer->x, layer->y, 
          layer->x + layer->w_size, layer->y + layer->h_size, height + 1);
      layers_refresh_sub(layer_mgr, layer->x, layer->y, 
          layer->x + layer->w_size, layer->y + layer->h_size, 
          height + 1, old_height);
    }
    else {
      if (layer_mgr->top > old_height) {
        for (h = old_height; h < layer_mgr->top; ++h) {
          layer_mgr->layers_addr[h] = layer_mgr->layers_addr[h + 1];
          layer_mgr->layers_addr[h]->height = h;
        }
      }
      --layer_mgr->top;
      layer_refresh_map(layer->mgr, layer->x, layer->y, 
          layer->x + layer->w_size, layer->y + layer->h_size, 0);
      layers_refresh_sub(layer_mgr, layer->x, layer->y, 
          layer->x + layer->w_size, layer->y + layer->h_size, 
          0, old_height - 1);
    }
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

    layer_refresh_map(layer_mgr, layer->x, layer->y, 
        layer->x + layer->w_size, layer->y + layer->h_size, height);
    layers_refresh_sub(layer_mgr, layer->x, layer->y, 
        layer->x + layer->w_size, layer->y + layer->h_size, height, height);
  }
}

void 
layers_refresh(layer_t* layer, int x0, int y0, int x1, int y1)
{
  if (layer->height >= 0) {
    layers_refresh_sub(layer->mgr, layer->x + x0, 
        layer->y + y0, layer->x + x1, layer->y + y1, 
        layer->height, layer->height);
  }
}


void 
layer_slide(layer_t* layer, int x, int y)
{
  int old_x = layer->x;
  int old_y = layer->y;
  layer->x = x;
  layer->y = y;
  if (layer->height >= 0) {
    layer_refresh_map(layer->mgr, old_x, old_y, 
        old_x + layer->w_size, old_y + layer->h_size, 0);
    layer_refresh_map(layer->mgr, x, y, 
        x + layer->w_size, y + layer->h_size, layer->height);
    layers_refresh_sub(layer->mgr, old_x, old_y, 
        old_x + layer->w_size, old_y + layer->h_size, 0, layer->height - 1);
    layers_refresh_sub(layer->mgr, x, y, 
        x + layer->w_size, y + layer->h_size, layer->height, layer->height);
  }
}
