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

BIN	= ipl.bin
SYS	= toyos.sys
IMG	= toyos.img
OSPATH	= ../tolset/z_tools/qemu
TOOLPATH = ../tolset/z_tools
NASK	= $(TOOLPATH)/nask.exe
EDIMG	= $(TOOLPATH)/edimg.exe
IMGTOOL	= $(TOOLPATH)/imgtol.exe
FDIMG0AT = $(TOOLPATH)/fdimg0at.tek
IPL_SRC	= ipl.s
OS_SRC	= toyos.s
IPL_LST	= ipl.lst
OS_LST	= toyos.lst
RM	= rm 
CP	= cp 
MAKE	= make
IMG_DEST = $(OSPATH)/fdimage0.bin


img: $(IMG)

asm: $(BIN)

install:
	$(CP) $(IMG) $(IMG_DEST)

uninstall:
	$(RM) $(IMG_DEST)

run:
	$(MAKE) -C $(OSPATH)

clean:
	$(RM) $(BIN) $(IMG) $(SYS) $(IPL_LST) $(OS_LST)


$(IMG): $(BIN) $(SYS)
	$(EDIMG) imgin:$(FDIMG0AT) wbinimg src:$(BIN) len:512 from:0 to:0\
		copy from:$(SYS) to:@: imgout:$(IMG)

$(BIN): $(IPL_SRC)
	$(NASK) $(IPL_SRC) $(BIN) $(IPL_LST)

$(SYS): $(OS_SRC)
	$(NASK) $(OS_SRC) $(SYS) $(OS_LST)
