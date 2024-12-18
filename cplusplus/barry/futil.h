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
#ifndef __FUTIL_HEADER_H__
#define __FUTIL_HEADER_H__

#include <stdio.h>
#include <sys/stat.h>

#ifdef _WIN32
# define FU_OPEN_R "rb"
# define FU_OPEN_W "wb"
# define FU_OPEN_RW "rwb"
#else
# define FU_OPEN_R "r"
# define FU_OPEN_W "w"
# define FU_OPEN_RW "rw"
#endif

typedef struct stat FUStats;

extern void fu_Error(const char* prefix);
extern FILE* fu_Open(const char* path, const char* flags);
extern int fu_Close(FILE* stream);
extern int fu_Rename(const char* from, const char* to);
extern FUStats* fu_Stat(const char* path);
extern FUStats* fu_Fstat(FILE* stream);
extern FUStats* fu_Lstat(const char* path);
extern int fu_Ftruncate(FILE* stream, int length);
extern int fu_Truncate(const char* path, int length);
extern int fu_Chown(const char* path, int uid, int gid);
extern int fu_Fchown(FILE* stream, int uid, int gid);
extern int fu_Lchown(const char* path, int uid, int gid);
extern size_t fu_Size(const char* path);
extern size_t fu_Fsize(FILE* stream);
extern char* fu_Read(const char* path);
extern char* fu_Nread(const char* path, int length);
extern char* fu_Fread(FILE* stream);
extern char* fu_Fnread(FILE* stream, int length);
extern int fu_Write(const char* path, const char* buffer);
extern int fu_Nwrite(const char* path, const char* buffer, int length);
extern int fu_Fwrite(FILE* stream, const char* buffer);
extern int fu_Fnwrite(FILE* stream, const char* buffer, int length);
extern int fu_Mkdir(const char* path, int mode);
extern int fu_Rmdir(const char* path);
extern int fu_Exists(const char* path);

#endif  /* __FUTIL_HEADER_H__ */
