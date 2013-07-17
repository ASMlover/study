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

TARGET	= liblua
BIN_OUT	= $(TARGET).dll
IMP_LIB	= $(TARGET).lib
BIN_DIR	= ..\bin 
CC	= cl -c -nologo
LINK	= link -nologo -dll -def:$(TARGET).def
MT	= mt -nologo 
CFLAGS	= -O2 -W3 -MT -GS -Zi -Fd"vc.pdb" -DNDEBUG
INCLUDES = -I"..\sdk\include"
LDDIRS	= -LIBPATH:"..\sdk\lib"
LDFLAGS	= -DEBUG -PDB:$(TARGET).pdb -manifest\
	-manifestfile:$(BIN_OUT).manifest -manifestuac:no $(LDDIRS) lua5.1.lib 
RM	= del 
MD	= mkdir 
RD	= rd /s /q
CP	= copy 
LIBLUA_OBJS = liblua.obj 




all: bin 

bin: $(BIN_OUT)

rebuild: clean all 

install:
	if not exist $(BIN_DIR) $(MD) $(BIN_DIR)
	$(CP) $(BIN_OUT) $(BIN_DIR)

uninstall:
	if exist $(BIN_DIR) $(RD) $(BIN_DIR)

clean:
	$(RM) $(BIN_OUT) $(IMP_LIB) $(LIBLUA_OBJS) *.ilk *.pdb *.manifest *.exp 





$(BIN_OUT): $(LIBLUA_OBJS)
	$(LINK) -out:$(BIN_OUT) $(LIBLUA_OBJS) $(LDFLAGS)
	$(MT) -manifest $(BIN_OUT).manifest -outputresource:$(BIN_OUT);1

.c.obj:
	$(CC) $(CFLAGS) $(INCLUDES) $<
