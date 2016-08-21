/*
 * Copyright (c) 2016 ASMlover. All rights reserved.
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
#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "futil.h"

void
fu_Error(const char* prefix)
{
  char buf[256];
  sprintf(buf, "fu: %s: error", prefix);
  perror(buf);
}

FILE*
fu_Open(const char* path, const char* flags)
{
  return fopen(path, flags);
}

int
fu_Close(FILE* stream)
{
  return fclose(stream);
}

int
fu_Rename(const char* from, const char* to)
{
  return rename(from, to);
}

FUStats*
fu_Stat(const char* path)
{
  FUStats* stats = (FUStats*)malloc(sizeof(FUStats));
  int e = stat(path, stats);
  if (-1 == e) {
    free(stats);
    return NULL;
  }
  return stats;
}

FUStats*
fu_Fstat(FILE* stream)
{
  if (NULL == stream) {
    return NULL;
  }
  else{
    FUStats* stats = (FUStats*)malloc(sizeof(FUStats));
    int fd = fileno(stream);
    int e = fstat(fd, stats);
    if (-1 == e) {
      free(stats);
      return NULL;
    }
    return stats;
  }
}

FUStats*
fu_Lstat(const char* path)
{
  FUStats* stats = (FUStats*)malloc(sizeof(FUStats));
#ifdef _WIN32
  int e = stat(path, stats);
#else
  int e = lstat(path, stats);
#endif
  if (-1 == e) {
    free(stats);
    return NULL;
  }
  return stats;
}

int
fu_Ftruncate(FILE* stream, int length)
{
  int fd = fileno(stream);
  return ftruncate(fd, (off_t)length);
}

int
fu_Truncate(const char* path, int length)
{
#ifdef _WIN32
  int ret = -1;
  int fd = open(path, O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
  if (-1 != fd) {
    ret = ftruncate(fd, (off_t)length);
    close(fd);
  }
  return ret;
#else
  return truncate(path, (off_t)length);
#endif
}

int
fu_Chown(const char* path, int uid, int gid)
{
#ifdef _WIN32
  errno = ENOSYS;
  return -1;
#else
  return chown(path, (uid_t)uid, (gid_t)gid);
#endif
}

int
fu_Fchown(FILE* stream, int uid, int gid)
{
#ifdef _WIN32
  errno = ENOSYS;
  return -1;
#else
  int fd = fileno(stream);
  return fchown(fd, (uid_t)uid, (gid_t)gid);
#endif
}

int
fu_Lchown(const char* path, int uid, int gid)
{
#ifdef _WIN32
  errno = ENOSYS;
  return -1;
#else
  return lchown(path, (uid_t)uid, (gid_t)gid);
#endif
}

size_t
fu_Size(const char* path)
{
  size_t size;
  FILE* stream = fu_Open(path, FU_OPEN_R);
  if (NULL == stream)
    return -1;
  fseek(stream, 0, SEEK_END);
  size = ftell(stream);
  fu_Close(stream);
  return size;
}

size_t
fu_Fsize(FILE* stream)
{
  size_t size;
  unsigned long pos = ftell(stream);
  rewind(stream);
  fseek(stream, 0, SEEK_END);
  size = ftell(stream);
  fseek(stream, pos, SEEK_SET);
  return size;
}

char*
fu_Read(const char* path)
{
  char* buffer;
  FILE* stream = fu_Open(path, FU_OPEN_R);
  if (NULL == stream)
    return NULL;
  buffer = fu_Fread(stream);
  fu_Close(stream);
  return buffer;
}

char*
fu_Nread(const char* path, int length)
{
  char* buffer;
  FILE* stream = fu_Open(path, FU_OPEN_R);
  if (NULL == stream)
    return NULL;
  buffer = fu_Fnread(stream, length);
  fu_Close(stream);
  return buffer;
}

char*
fu_Fread(FILE* stream)
{
  size_t fsize = fu_Fsize(stream);
  return fu_Fnread(stream, fsize);
}

char*
fu_Fnread(FILE* stream, int length)
{
  char* buffer = (char*)malloc(sizeof(char) * (length + 1));
  size_t n = fread(buffer, 1, length, stream);
  buffer[n] = 0;
  return buffer;
}

int
fu_Write(const char* path, const char* buffer)
{
  return fu_Nwrite(path, buffer, strlen(buffer));
}

int
fu_Nwrite(const char* path, const char* buffer, int length)
{
  int count;
  FILE* stream = fu_Open(path, FU_OPEN_W);
  if (NULL == stream)
    return -1;
  count = fu_Fnwrite(stream, buffer, length);
  fu_Close(stream);
  return count;
}

int
fu_Fwrite(FILE* stream, const char* buffer)
{
  return fu_Fnwrite(stream, buffer, strlen(buffer));
}

int
fu_Fnwrite(FILE* stream, const char* buffer, int length)
{
  return (int)fwrite(buffer, 1, length, stream);
}

int
fu_Mkdir(const char* path, int mode)
{
#ifdef _WIN32
  return mkdir(path);
#else
  return mkdir(path, (mode_t)mode);
#endif
}

int
fu_Rmdir(const char* path)
{
  return rmdir(path);
}

int
fu_Exists(const char* path)
{
  struct stat s;
  return stat(path, &s);
}
