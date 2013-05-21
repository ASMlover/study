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
#ifndef __GRAPHIC_HEADER_H__
#define __GRAPHIC_HEADER_H__

/* graphic */
#define COLOR8_000000   (0)
#define COLOR8_FF0000   (1)
#define COLOR8_00FF00   (2)
#define COLOR8_FFFF00   (3)
#define COLOR8_0000FF   (4)
#define COLOR8_FF00FF   (5)
#define COLOR8_00FFFF   (6)
#define COLOR8_FFFFFF   (7)
#define COLOR8_C6C6C6   (8)
#define COLOR8_840000   (9)
#define COLOR8_008400   (10)
#define COLOR8_848400   (11)
#define COLOR8_000084   (12)
#define COLOR8_840084   (13)
#define COLOR8_008484   (14)
#define COLOR8_848484   (15)

extern void init_palette(void);
extern void fill_box8(unsigned char* vram, int xsize, 
    unsigned char color, int left, int top, int right, int bottom);
extern void init_screen(unsigned char* vram, int w, int h);
extern void draw_font8(unsigned char* vram, 
    int xsize, int x, int y, char c, char* font);
extern void draw_font8_asc(unsigned char* vram, int xsize, 
    int x, int y, char color, unsigned char* string);
extern void init_mouse_cursor8(char* mouse, char bg_color);
extern void draw_block8_8(char* vram, int vxsize, int mouse_w, 
    int mouse_h, int x, int y, char* img_addr, int pixels_per_line);

#endif  /* __GRAPHIC_HEADER_H__ */
