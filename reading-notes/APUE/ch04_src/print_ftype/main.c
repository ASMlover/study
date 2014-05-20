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
#include <sys/stat.h>
#include <stdio.h>


int main(int argc, char* argv[]) {
  int i;
  struct stat st;
  char* ptr;

  for (i = 1; i < argc; ++i) {
    fprintf(stdout, "%s: ", argv[i]);
    if (lstat(argv[i], &st) < 0) {
      fprintf(stderr, "lstat error\n");
      continue;
    }

    if (S_ISREG(st.st_mode)) 
      ptr = "regular";
    else if (S_ISDIR(st.st_mode))
      ptr = "directory";
    else if (S_ISCHR(st.st_mode))
      ptr = "character special";
    else if (S_ISBLK(st.st_mode))
      ptr = "block special";
    else if (S_ISFIFO(st.st_mode))
      ptr = "fifo";
    else if (S_ISLNK(st.st_mode))
      ptr = "symbolic link";
    else if (S_ISSOCK(st.st_mode))
      ptr = "socket";
    else 
      ptr = "** unknown mode **";

    fprintf(stdout, "%s\n", ptr);
  }

  return 0;
}
