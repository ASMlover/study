# Copyright (c) 2015 ASMlover. All rights reserved.
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

OUT	= elang.exe
RM	= del
CC	= cl -c -nologo
MT	= mt -nologo
LINK	= link -nologo
CFLAGS	= -O2 -W3 -MDd -GS -Zi -Fd"vc.pdb" -EHsc -D_DEBUG -DEL_DEBUG\
	-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -wd4290
LDFLAGS	= -INCREMENTAL -DEBUG -PDB:$(OUT).pdb -manifest\
	-manifestfile:$(OUT).manifest -manifestuac:no
OBJS	= main.obj\
	\
	el_string.obj el_string_table.obj el_token.obj el_lexer.obj\
	el_line_normalizer.obj el_parser_base.obj


all: $(OUT)

rebuild: clean all

clean:
	$(RM) $(OUT) $(OBJS) *.pdb *.ilk *.manifest


$(OUT): $(OBJS)
	$(LINK) -out:$(OUT) $(OBJS) $(LDFLAGS)
	$(MT) -manifest $(OUT).manifest -outputresource:$(OUT);1

.cc.obj:
	$(CC) $(CFLAGS) $<

{.\eLang}.cc{}.obj:
	$(CC) $(CFLAGS) $<
