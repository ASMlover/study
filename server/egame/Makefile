# Copyright (c) 2014 ASMlover. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list ofconditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materialsprovided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

OUT	= egame
RM	= rm 
CC	= g++
INCLUDES = -I./sdk/libnet -I./sdk/hiredis/inc -I./utilities
CPPFLAGS = -g -O2 -Wall -std=c++0x
CFLAGS	= -g -O2 -Wall
LDFLAGS	= -lpthread -L./sdk/hiredis/lib -lhiredis
CPP_OBJS = main.o player.o player_mgr.o user_cache.o config.o
C_OBJS	= ./sdk/libnet/libnet.o





all: $(OUT)

rebuild: clean all 

clean:
	$(RM) $(OUT) $(CPP_OBJS) $(C_OBJS)





$(OUT): $(CPP_OBJS) $(C_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(CPP_OBJS): %.o: %.cc 
	$(CC) -o $*.o -c $(CPPFLAGS) $(INCLUDES) $^

$(C_OBJS): %.o: %.c
	$(CC) -o $*.o -c $(CFLAGS) $^
