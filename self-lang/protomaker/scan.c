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
#include "globals.h"
#include "scan.h"


enum ScanStatus {
  SCAN_STATUS_START = 0, 
  SCAN_STATUS_DONE, 

  SCAN_STATUS_IN_ACCESS, 
  SCAN_STATUS_IN_ID, 
  SCAN_STATUS_IN_CINT, 
  SCAN_STATUS_IN_CREAL, 
  SCAN_STATUE_IN_COMMENT, 
};


static const struct {
  char* str;
  int   tok;
} kReserveds[] = {
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


#define MAX_LINE_BUF    (512)
static char s_line_buf[MAX_LINE_BUF];
static int  s_line_pos = 0;
static int  s_buf_size = 0;
static int  s_eof = BOOL_NO;




static void 
echo_scanner(FILE* stream, int lineno, int type, const char* token)
{
  fprintf(stream, "        %d: ", lineno);

  switch (type) {
  case TOKEN_TYPE_DEFAULT:
  case TOKEN_TYPE_ENUM:
  case TOKEN_TYPE_MESSAGE:
  case TOKEN_TYPE_PROTOCOL:
  case TOKEN_TYPE_TYPE:
  case TOKEN_TYPE_BYTE:
  case TOKEN_TYPE_INT8:
  case TOKEN_TYPE_UINT8:
  case TOKEN_TYPE_INT16:
  case TOKEN_TYPE_UINT16:
  case TOKEN_TYPE_INT32:
  case TOKEN_TYPE_UINT32:
  case TOKEN_TYPE_INT64:
  case TOKEN_TYPE_UINT64:
  case TOKEN_TYPE_REAL32:
  case TOKEN_TYPE_REAL64:
    fprintf(stream, "keyword: %s\n", token);
    break;
  case TOKEN_TYPE_ASSIGN:
    fprintf(stream, "=\n");
    break;
  case TOKEN_TYPE_INHERIT:
    fprintf(stream, "<\n");
    break;
  case TOKEN_TYPE_ACCESS:
    fprintf(stream, ".\n");
    break;
  case TOKEN_TYPE_LBRACKET:
    fprintf(stream, "[\n");
    break;
  case TOKEN_TYPE_RBRACKET:
    fprintf(stream, "]\n");
    break;
  case TOKEN_TYPE_LBRACE:
    fprintf(stream, "{\n");
    break;
  case TOKEN_TYPE_RBRACE:
    fprintf(stream, "}\n");
    break;
  case TOKEN_TYPE_EOF:
    fprintf(stream, "EOF\n");
    break;
  case TOKEN_TYPE_CINT:
    fprintf(stream, "NUM(INT), value => %s\n", token);
    break;
  case TOKEN_TYPE_CREAL:
    fprintf(stream, "NUM(REAL), value => %s\n", token);
    break;
  case TOKEN_TYPE_ID:
    fprintf(stream, "ID, name => %s\n", token);
    break;
  case TOKEN_TYPE_ERR:
    fprintf(stream, "ERROR: %s\n", token);
    break;
  default:
    fprintf(stream, "Unknown token: %d\n", type);
  }
}




static int 
get_char(void)
{
  if (s_line_pos >= s_buf_size) {
    if (NULL != fgets(s_line_buf, MAX_LINE_BUF - 1, g_source_stream)) {
      fprintf(g_scan_stream, "%4d: %s", g_line_numer, s_line_buf);

      s_buf_size = (int)strlen(s_line_buf);
      s_line_pos = 0;
      return s_line_buf[s_line_pos++];
    }
    else {
      s_eof = BOOL_YES;
      return EOF;
    }
  }

  return s_line_buf[s_line_pos++];
}

static void 
unget_char(void)
{
  if (!s_eof)
    --s_line_pos;
}


static int 
lookup_reserved(const char* s) 
{
  int i;
  int count = countof(kReserveds);
  for (i = 0; i < count; ++i) {
    if (0 == strcmp(s, kReserveds[i].str))
      return kReserveds[i].tok;
  }

  return TOKEN_TYPE_ID;
}


int 
get_token(void)
{
  int type = TOKEN_TYPE_ERR;
  int index = 0;
  int status = SCAN_STATUS_START;
  int save;

  int c;
  while (status != SCAN_STATUS_DONE) {
    c = get_char();
    save = BOOL_YES;

    switch (status) {
    case SCAN_STATUS_START:
      if (' ' == c || '\t' == c) {
        save = BOOL_NO;
      }
      else if ('\n' == c) {
        save = BOOL_NO;
        ++g_line_numer;
      }
      else if (isdigit(c)) {
        status = SCAN_STATUS_IN_CINT;
      }
      else if (isalpha(c) || '_' == c) {
        status = SCAN_STATUS_IN_ID;
      }
      else if ('.' == c) {
        status = SCAN_STATUS_IN_ACCESS;
      }
      else if ('#' == c) {
        status = SCAN_STATUE_IN_COMMENT;
      }
      else {
        status = SCAN_STATUS_DONE;
        switch (c) {
        case EOF:
          save = BOOL_NO;
          type = TOKEN_TYPE_EOF;
          break;
        case '=':
          type = TOKEN_TYPE_ASSIGN;
          break;
        case '<':
          type = TOKEN_TYPE_INHERIT;
          break;
        case '[':
          type = TOKEN_TYPE_LBRACKET;
          break;
        case ']':
          type = TOKEN_TYPE_RBRACKET;
          break;
        case '{':
          type = TOKEN_TYPE_LBRACE;
          break;
        case '}':
          type = TOKEN_TYPE_RBRACE;
          break;
        default:
          save = BOOL_NO;
          type = TOKEN_TYPE_ERR;
          break;
        }
      }
      break;
    case SCAN_STATUS_IN_ACCESS:
      if (isalpha(c) || '_' == c) {
        unget_char();
        save = BOOL_NO;
        status = SCAN_STATUS_DONE;
        type = TOKEN_TYPE_ACCESS;
      }
      else {
        fprintf(stderr, "Lexial error: [%d] after '.' ...\n", g_line_numer);
        exit(1);
      }
      break;
    case SCAN_STATUS_IN_ID:
      if (!isalnum(c) && '_' != c) {
        unget_char();
        save = BOOL_NO;
        status = SCAN_STATUS_DONE;
        type = TOKEN_TYPE_ID;
      }
      break;
    case SCAN_STATUS_IN_CINT:
      if ('.' == c) {
        status = SCAN_STATUS_IN_CREAL;
      }
      else {
        if (!isdigit(c)) {
          unget_char();
          save = BOOL_NO;
          status = SCAN_STATUS_DONE;
          type = TOKEN_TYPE_CINT;
        }
      }
      break;
    case SCAN_STATUS_IN_CREAL:
      if (!isdigit(c)) {
        unget_char();
        save = BOOL_NO;
        status = SCAN_STATUS_DONE;
        type = TOKEN_TYPE_CREAL;
      }
      break;
    case SCAN_STATUE_IN_COMMENT:
      save = BOOL_NO;
      if (EOF == c) {
        status = SCAN_STATUS_DONE;
        type = TOKEN_TYPE_EOF;
      }
      else if ('\n' == c) {
        ++g_line_numer;
        status = SCAN_STATUS_START;
      }
      break;
    case SCAN_STATUS_DONE:
    default:
      fprintf(g_scan_stream, "Scanner bug: status = %d\n", status);
      status = SCAN_STATUS_DONE;
      type = TOKEN_TYPE_ERR;
      break;
    }

    if (save && index < MAX_TOKEN)
      g_token[index++] = (char)c;

    if (SCAN_STATUS_DONE == status) {
      g_token[index] = 0;
      if (TOKEN_TYPE_ID == type)
        type = lookup_reserved(g_token);
    }
  }

  echo_scanner(g_scan_stream, g_line_numer, type, g_token);

  return type;
}
