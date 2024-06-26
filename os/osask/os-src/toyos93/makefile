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

IMG	= toyos.img
OSPATH	= ../tolset/z_tools/qemu
TOOLPATH = ../tolset/z_tools
INCPATH	= ../tolset/z_tools/haribote/
NASK	= $(TOOLPATH)/nask.exe
CC	= $(TOOLPATH)/cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)/gas2nask.exe -a
OBJ2BIM	= $(TOOLPATH)/obj2bim.exe
MAKEFONT = $(TOOLPATH)/makefont.exe
BIN2OBJ	= $(TOOLPATH)/bin2obj.exe
BIM2HRB	= $(TOOLPATH)/bim2hrb.exe
RULEFILE = $(TOOLPATH)/haribote/haribote.rul
EDIMG	= $(TOOLPATH)/edimg.exe
IMGTOOL	= $(TOOLPATH)/imgtol.exe
FDIMG0AT = $(TOOLPATH)/fdimg0at.tek
OBJ_BIN	= ipl.bin
OBJ_SYS	= toyos.sys 
OBJS	= bootpack.obj common.obj charlib.obj graphic.obj desc_tbl.obj\
	pic.obj fifo.obj keyboard.obj mouse.obj memory.obj layer.obj timer.obj\
	multitask.obj
RM	= rm 
CP	= cp 
MAKE	= make
IMG_DEST = $(OSPATH)/fdimage0.bin 
TEMP_FILE = *.bin *.lst *.obj bootpack.map bootpack.bim\
	bootpack.hrb toyos.sys


img: $(IMG)

install:
	$(CP) $(IMG) $(IMG_DEST)

uninstall:
	$(RM) $(IMG_DEST)

run:
	$(MAKE) -C $(OSPATH)

clean:
	$(RM) $(IMG) $(TEMP_FILE)


$(IMG): $(OBJ_BIN) $(OBJ_SYS)
	$(EDIMG) imgin:$(FDIMG0AT) wbinimg src:$(OBJ_BIN) len:512 from:0 to:0\
		copy from:$(OBJ_SYS) to:@: \
		copy from:toyos.s to:@: \
		copy from:makefile to:@: \
		imgout:$(IMG)

ipl.bin: ipl.s
	$(NASK) ipl.s ipl.bin ipl.lst 

toyos.sys: toyos.bin bootpack.hrb
	cat toyos.bin bootpack.hrb > toyos.sys 

toyos.bin: toyos.s
	$(NASK) toyos.s toyos.bin toyos.lst

bootpack.hrb: bootpack.bim 
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

bootpack.bim: $(OBJS)
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS)

charlib.obj: charlib.bin 
	$(BIN2OBJ) charlib.bin charlib.obj _charlib

charlib.bin: charlib.txt
	$(MAKEFONT) charlib.txt charlib.bin


%.gas: %.c
	$(CC) -o $*.gas $*.c

%.s: %.gas
	$(GAS2NASK) $*.gas $*.s

%.obj: %.s
	$(NASK) $*.s $*.obj $*.lst
