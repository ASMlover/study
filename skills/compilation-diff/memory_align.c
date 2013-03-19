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
#include <stdio.h>
#ifdef _WIN32
  typedef signed __int64  int64_t;
#else 
  #include <stdint.h>
#endif


#define SHOW_SZ(t)\
  fprintf(stdout, "sizeof(" #t ") = %d\n", sizeof(t))


/*
 * 当一个结构体中的所有成员都是1字节的时候, 该结构的成员都是以1字节对齐的, 
 * 而结构体的大小就是其所有成员大小之和, 而无论程序中所设置的对齐方式到底是
 * 多少, 其结构的大小没有影响
 *
 * 3 bytes
 */
typedef struct demo1_s {
  char c1;
  char c2;
  char c3;
} demo1_t;
#pragma pack(push, 8)
typedef struct demo2_s {
  char c1;
  char c2;
  char c3;
} demo2_t;
#pragma pack(pop) 


/*
 * 对于一个结构前面的成员都很小, 而后面的成员是个大字节的成员的情况下, 在遇
 * 到这个大字节成员之前都按照原来相应的成员字节数最大的那个成员来对齐; 而与
 * 这个大字节成员相遇后则需要按照该成员来对齐
 *
 * 4 bytes
 */
typedef struct demo3_s {
  char c1;
  short s1;
} demo3_t; 


/*
 * 对于结构前面的成员按照结构中最大的那个成员对齐之后, 在该最大的成员之后又
 * 有小于该成员的成员时, 这个时候结构体的对齐应该将这个成员按照结构体中最大
 * 的那个成员来对齐
 *
 * 24 bytes
 */
typedef struct demo4_s {
  char c1;
  short s1;
  int i1;
  int64_t i64;
  char c2;
} demo4_t;


/*
 * 对于最大成员之后的成员, 这个时候结构已经对齐了; 后面的成员对齐则按照类似
 * 从结构体第0字节开始的对齐方式来对齐;
 * 而结构体的对齐, 则会自动按照最大成员来补齐
 *
 * 去掉i1, s1, c2, c3中任何一个, 都为24 bytes
 */
typedef struct demo5_s {
  char c1;
  int64_t i54;
  int i1;
  short s1;
  char c2;
  char c3;
} demo5_t;


int 
main(int argc, char* argv[])
{
  SHOW_SZ(demo1_t);
  SHOW_SZ(demo2_t);
  SHOW_SZ(demo3_t);
  SHOW_SZ(demo4_t);
  SHOW_SZ(demo5_t);

  return 0;
}
