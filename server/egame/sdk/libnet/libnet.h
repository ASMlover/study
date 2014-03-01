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
#ifndef __LIBNET_HEADER_H__
#define __LIBNET_HEADER_H__

#if defined(_WINDOWS_) || defined(_MSC_VER)
# if (_MSC_VER < 1600)
#   if (_MSC_VER < 1300)
      typedef signed char       int8_t;
      typedef unsigned char     uint8_t;
      typedef signed short      int16_t;
      typedef unsigned short    uint16_t;
      typedef signed int        int32_t;
      typedef unsigned int      uint32_t;
#   else
      typedef signed __int8     int8_t;
      typedef unsigned __int8   uint8_t;
      typedef signed __int16    int16_t;
      typedef unsigned __int16  uint16_t;
      typedef signed __int32    int32_t;
      typedef unsigned __int32  uint32_t;
#   endif
    typedef signed __int64      int64_t;
    typedef unsigned __int64    uint64_t;
# else 
#   include <stdint.h>
# endif
#elif defined(__linux__) || defined(__GNUC__)
# include <stdint.h>
#endif


#ifdef __cplusplus
  extern "C" {
#endif


/*
 * type of net message from remote
 */
enum MessageType {
  MESSAGETYPE_CONNECT     = 0,
  MESSAGETYPE_DISCONNECT  = 1, 
  MESSAGETYPE_DATA        = 2, 
};

#define INVAL_CONNECTOR   (-1)

typedef struct NetMessage {
  uint32_t  connid; /* ID of connector */
  int       type;   /* type of net message */
  uint32_t  size;   /* size of net message */
  uint8_t*  data;   /* real net message data */
} NetMessage;

typedef struct NetHandler NetHandler;


extern NetHandler* net_create(
    int worker_num, int max_client, 
    int read_buf, int write_buf);
extern void net_destroy(NetHandler** handler);

extern int net_listen(
    NetHandler* handler, const char* host_addr, uint16_t port);
extern uint32_t net_connect(
    NetHandler* handler, const char* remote_addr, uint16_t port);

extern int net_read(NetHandler* handler, NetMessage* msg);
extern int net_write(NetHandler* handler, 
    uint32_t connid, const void* data, uint32_t size);

extern void net_msg_free(NetMessage* msg);



#ifdef __cplusplus
  }
#endif

#endif  /* __LIBNET_HEADER_H__ */
