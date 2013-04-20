# Copyright (c) 2013 ASMlover. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list ofconditions and the following disclaimer.
#
#    notice, this list of conditions and the following disclaimer in
#  * Redistributions in binary form must reproduce the above copyright
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

BIN	= helloos.bin
IMG	= helloos.img
NASK	= ../tolset/z_tools/nask.exe
EDIMG	= ../tolset/z_tools/edimg.exe 
FDIMG0AT = ../tolset/z_tools/fdimg0at.tek
SRC	= helloos.asm
RM	= rm 
CP	= cp
IMG_DEST = ../tolset/z_tools/qemu/fdimage0.bin


img: $(IMG)

asm: $(BIN)

install:
	$(CP) $(IMG) $(IMG_DEST)

uninstall:
	$(RM) $(IMG_DEST)

run:
	make -C ../tolset/z_tools/qemu

clean:
	$(RM) $(BIN) $(IMG) helloos.lst


$(IMG): $(BIN)
	$(EDIMG) imgin:$(FDIMG0AT) wbinimg src:$(BIN) len:512 from:0 to:to\
		imgout:$(IMG)

$(BIN): $(SRC)
	$(NASK) $(SRC) $(BIN) helloos.lst
