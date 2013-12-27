# Copyright (c) 2013 ASMlover. All rights reserved.
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

OUT	= demo 
RM	= rm 
CC	= g++ 
CFLAGS	= -g -O2 -Wall 
LDFLAGS	= -lpthread -lrt
OBJS	= main.o server.o client.o\
	\
	../posix/el_posix_condition.o ../posix/el_posix_io.o\
	../posix/el_posix_file.o ../posix/el_posix_time.o\
	../posix/el_posix_net.o ../posix/el_posix_socket.o\
	../posix/el_posix_epoll.o\
	\
	../el_allocator.o ../el_thread_pool.o ../el_circular_buffer.o\
	../el_time.o ../el_logging.o ../el_net.o ../el_net_buffer.o\
	../el_address.o ../el_socket.o ../el_connector.o\
	../el_connector_dispatcher.o ../el_net_worker.o ../el_net_listener.o\
	../el_network_handler.o




all: $(OUT)

rebuild: clean all 

clean:
	$(RM) $(OUT) $(OBJS)




$(OUT): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): %.o: %.cc 
	$(CC) -o %*.o -c $(CFLAGS) $^
