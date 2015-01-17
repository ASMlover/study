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

CLT_OUT	= echo_client.exe
SRV_OUT	= echo_server.exe
OUT	= $(CLT_OUT) $(SRV_OUT)
RM	= del
CC	= cl -c -nologo
MT	= mt -nologo
LINK	= link -nologo
CFLAGS	= -O2 -W3 -MD -GS -Zi -Fd"vc.pdb" -D_DEBUG\
	-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS
CLT_LDFLAGS	= -INCREMENTAL -DEBUG -PDB:$(CLT_OUT).pdb -manifest\
	-manifestfile:$(CLT_OUT).manifest -manifestuac:no\
	-LIBPATH:"..\common" ws2_32.lib libcommon.lib 
SRV_LDFLAGS	= -INCREMENTAL -DEBUG -PDB:$(SRV_OUT).pdb -manifest\
	-manifestfile:$(SRV_OUT).manifest -manifestuac:no\
	-LIBPATH:"..\common" ws2_32.lib libcommon.lib 
CLT_OBJS	= echo_client.obj
SRV_OBJS	= echo_server.obj
OBJS	= $(CLT_OBJS) $(SRV_OBJS)

all: $(OUT)
rebuild: clean all
clean:
	$(RM) $(OUT) $(OBJS) *.pdb *.manifest *.ilk

$(CLT_OUT): $(CLT_OBJS)
	$(LINK) -out:$(CLT_OUT) $(CLT_OBJS) $(CLT_LDFLAGS)
	$(MT) -manifest $(CLT_OUT).manifest -outputresource:$(CLT_OUT);1
$(SRV_OUT): $(SRV_OBJS)
	$(LINK) -out:$(SRV_OUT) $(SRV_OBJS) $(SRV_LDFLAGS)
	$(MT) -manifest $(SRV_OUT).manifest -outputresource:$(SRV_OUT);1
.c.obj:
	$(CC) $(CFLAGS) -I"..\common" $<
