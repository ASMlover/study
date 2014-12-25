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
#ifndef __COMMON_HEADER_H__
#define __COMMON_HEADER_H__

#if defined(_WINDOWS_) || defined(_MSC_VER)
# define PLATFORM_WIN
#elif defined(__linux__)
# define PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
# define PLATFORM_MAC
#else
# error "Un-Support this platform!"
#endif

#if defined(PLATFORM_WIN)
# include <winsock2.h>
# include <process.h>

  typedef int              socklen_t;
  typedef CRITICAL_SECTION mutex_t;
#else
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/stat.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>
# include <fcntl.h>
# include <pthread.h>

  typedef pthread_mutex_t mutex_t;
#endif
#include <errno.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(PLATFORM_WIN)
# define snprintf _snprintf
#endif

extern void network_init(void);
extern void network_destroy(void);

extern int error_quit(const char* message);
extern int error_print(const char* message);

/* mutex locker module */
extern void mutex_init(mutex_t* mutex);
extern void mutex_destroy(mutex_t* mutex);
extern void mutex_lock(mutex_t* mutex);
extern void mutex_unlock(mutex_t* mutex);

/* socket network module */
extern int common_socket(int family, int type, int protocol);
extern void common_bind(int fd, 
    struct sockaddr* addr, socklen_t addrlen);
extern void common_listen(int fd, int backlog);
extern int common_accept(int fd, 
    struct sockaddr* addr, socklen_t* addrlen);
extern void common_connect(int fd, 
    struct sockaddr* addr, socklen_t addrlen);
extern int common_read(int fd, int buflen, char* buffer);
extern int common_write(int fd, const char* buffer, int buflen);
extern void common_close(int fd);

#endif  /* __COMMON_HEADER_H__ */
