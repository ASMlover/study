/*
 * Copyright (c) 2012 ASMlover. All rights reserved.
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
#if defined(_WINDOWS_) || defined(_MSC_VER)
# ifndef __LCC__
#   define __LCC__
# endif
#endif
#include <assert.h>
#include <stdio.h>
#include <mysql.h>
#include "mysqli.h"



struct mysqli_t {
  MYSQL*      mysql;
  MYSQL_RES*  result;
};




static void 
mysqli_error(MYSQL* mysql, const char* error_prompt)
{
  if (NULL != mysql) 
    fprintf(stderr, "%s - error[%d] : %s\n", 
        error_prompt, mysql_errno(mysql), mysql_error(mysql));
  else 
    fprintf(stderr, "%s\n", error_prompt);
}




void* 
mysqli_connect(const char* host, unsigned int port, 
               const char* user, const char* passwd, 
               const char* db, const char* charset)
{
  struct mysqli_t* mysqli = (struct mysqli_t*)calloc(1, sizeof(*mysqli));
  if (NULL == mysqli)
    return NULL;

  do {
    if (NULL == (mysqli->mysql = mysql_init(NULL))) {
      mysqli_error(NULL, "mysql_init failed");
      break;
    }

    if (0 != mysql_options(mysqli->mysql, 
          MYSQL_OPT_CONNECT_TIMEOUT, "120") 
        || 0 != mysql_options(mysqli->mysql, 
          MYSQL_SET_CHARSET_NAME, charset)) {
      mysqli_error(mysqli->mysql, "mysql_options");
      break;
    }

    if (NULL == mysql_real_connect(mysqli->mysql, 
          host, user, passwd, db, port, NULL, 0UL)) {
      mysqli_error(mysqli->mysql, "mysql_real_connect");
      break;
    }

    return mysqli;
  } while (0);

  if (NULL != mysqli->mysql)
    mysql_close(mysqli->mysql);
  free(mysqli);

  return NULL;
}

void 
mysqli_close(void** link_identifier)
{
  struct mysqli_t* mysqli = (struct mysqli_t*)*link_identifier;
  if (NULL != mysqli) {
    if (NULL != mysqli->result)
      mysql_free_result(mysqli->result);
    if (NULL != mysqli->mysql)
      mysql_close(mysqli->mysql);

    free(*link_identifier);
    *link_identifier = NULL;
  }
}

int 
mysqli_execute(void* link_identifier, const char* query)
{
  struct mysqli_t* mysqli = (struct mysqli_t*)link_identifier;
  int exec_ret = -1;

  if (NULL != mysqli && NULL != mysqli->mysql && NULL != query) {
    exec_ret = mysql_real_query(mysqli->mysql, query, strlen(query));
    if (0 == exec_ret) {
      if (NULL == (mysqli->result = mysql_store_result(mysqli->mysql))) {
        exec_ret = mysql_errno(mysqli->mysql);
        if (0 != exec_ret)
          mysqli_error(mysqli->mysql, "mysql_store_result");
      }
    } else {
      mysqli_error(mysqli->mysql, "mysql_real_query");
    }
  }

  return exec_ret;
}


int 
mysqli_fields(void* link_identifier)
{
  struct mysqli_t* mysqli = (struct mysqli_t*)link_identifier;
  return (NULL != mysqli && NULL != mysqli->result 
    ? mysql_num_fields(mysqli->result) 
    : 0);
}

uint64_t 
mysqli_rows(void* link_identifier)
{
  struct mysqli_t* mysqli = (struct mysqli_t*)link_identifier;
  return (NULL != mysqli && NULL != mysqli->result 
    ? mysql_num_rows(mysqli->result) 
    : 0);
}

char** 
mysqli_row(void* link_identifier)
{
  struct mysqli_t* mysqli = (struct mysqli_t*)link_identifier;
  return (NULL != mysqli && NULL != mysqli->result 
    ? mysql_fetch_row(mysqli->result) 
    : NULL);
}
