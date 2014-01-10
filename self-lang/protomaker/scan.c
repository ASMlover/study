/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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
#include <stdio.h>
#include "globals.h"
#include "scan.h"


enum ScanStatus {
  SCAN_STATUS_START = 0, 
  SCAN_STATUS_IN_ACCESS, 
  SCAN_STATUS_IN_NUMBER, 
  SCAN_STATUS_IN_ID, 
  SCAN_STATUS_DONE, 
};


static const struct {
  char* str;
  int   tok;
} kKeywords[] = {
  {"default", TOKEN_TYPE_DEFAULT}, 
  {"enum", TOKEN_TYPE_ENUM}, 
  {"message", TOKEN_TYPE_MESSAGE}, 
  {"protocol", TOKEN_TYPE_PROTOCOL}, 
  {"type", TOKEN_TYPE_TYPE}, 

  {"byte", TOKEN_TYPE_BYTE}, 
  {"int8", TOKEN_TYPE_INT8}, 
  {"uint8", TOKEN_TYPE_UINT8}, 
  {"int16", TOKEN_TYPE_INT16}, 
  {"uint16", TOKEN_TYPE_UINT16}, 
  {"int32", TOKEN_TYPE_INT32}, 
  {"uint32", TOKEN_TYPE_UINT32}, 
  {"int64", TOKEN_TYPE_INT64}, 
  {"uint64", TOKEN_TYPE_UINT64}, 
  {"real32", TOKEN_TYPE_REAL32}, 
  {"real64", TOKEN_TYPE_REAL64}, 
};
