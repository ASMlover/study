# Copyright (c) 2016 ASMlover. All rights reserved.
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

BIN = {bin}
RM = del
CC = cl -c -nologo
MT = mt -nologo
LINK = link -nologo
CFLAGS = -GS -Zi -Fd"vc.pdb" -EHsc{cflags}{preprocessor}{inc_dir}
LDFLAGS = -INCREMENTAL -DEBUG -PDB:$(BIN).pdb -manifest -manifestfile:$(BIN).manifest -manifestuac:no{ldflags}{lib_dir}{dep_libs}
OBJS = {objs}
SRCS = {srcs}

all: $(BIN)

rebuild: clean all

clean:
	$(RM) $(BIN) $(OBJS) *.pdb *.ilk *.manifest

$(BIN): $(OBJS)
	$(LINK) -out:$(BIN) $(OBJS) $(LDFLAGS)
	$(MT) -manifest $(BIN).manifest -outputresource:$(BIN);1

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)
