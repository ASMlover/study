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
	multitask.obj window.obj console.obj file.obj
RM	= rm 
CP	= cp 
MAKE	= make
IMG_DEST = $(OSPATH)/fdimage0.bin 
TEMP_FILE = *.bin *.lst *.obj *.map *.bim bootpack.hrb toyos.sys *.toy
APP	= hello.toy
APP_SRC	= hello.s
APP_LST	= hello.lst
APP2	= hello2.toy
APP_SRC2 = hello2.s
APP_LST2 = hello2.lst

img: $(IMG)

install:
	$(CP) $(IMG) $(IMG_DEST)

uninstall:
	$(RM) $(IMG_DEST)

run:
	$(MAKE) -C $(OSPATH)

clean:
	$(RM) $(IMG) $(TEMP_FILE)


$(IMG): $(OBJ_BIN) $(OBJ_SYS) $(APP) $(APP2) a.toy hello3.toy crack.toy \
	crack2.toy crack3.toy crack4.toy crack5.toy bug1.toy bug2.toy bug3.toy \
	hello4.toy hello5.toy
	$(EDIMG) imgin:$(FDIMG0AT) wbinimg src:$(OBJ_BIN) len:512 from:0 to:0\
		copy from:$(OBJ_SYS) to:@: \
		copy from:toyos.s to:@: \
		copy from:makefile to:@: \
		copy from:cat.txt to:@: \
		copy from:$(APP) to:@: \
		copy from:$(APP2) to:@: \
		copy from:a.toy to:@: \
		copy from:hello3.toy to:@: \
		copy from:crack.toy to:@: \
		copy from:crack2.toy to:@: \
		copy from:crack3.toy to:@: \
		copy from:crack4.toy to:@: \
		copy from:crack5.toy to:@: \
		copy from:bug1.toy to:@: \
		copy from:bug2.toy to:@: \
		copy from:bug3.toy to:@: \
		copy from:hello4.toy to:@: \
		copy from:hello5.toy to:@: \
		imgout:$(IMG)

##########################################################
##########################################################
# user application
$(APP): $(APP_SRC)
	$(NASK) $(APP_SRC) $(APP) $(APP_LST)

$(APP2): $(APP_SRC2)
	$(NASK) $(APP_SRC2) $(APP2) $(APP_LST2)

a.toy: a.bim
	$(BIM2HRB) a.bim a.toy 0

a.bim: a.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:a.bim map:a.map a.obj user_api.obj 

hello3.toy: hello3.bim
	$(BIM2HRB) hello3.bim hello3.toy 0

hello3.bim: hello3.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:hello3.bim map:hello3.map \
		hello3.obj user_api.obj 

crack.toy: crack.bim
	$(BIM2HRB) crack.bim crack.toy 0

crack.bim: crack.obj user_api.obj
	$(OBJ2BIM) @$(RULEFILE) out:crack.bim map:crack.map crack.obj user_api.obj

crack2.toy: crack2.s 
	$(NASK) crack2.s crack2.toy crack2.lst

crack3.toy: crack3.s
	$(NASK) crack3.s crack3.toy crack3.lst

crack4.toy: crack4.s
	$(NASK) crack4.s crack4.toy crack4.lst

crack5.toy: crack5.s
	$(NASK) crack5.s crack5.toy crack5.lst

bug1.toy: bug1.bim
	$(BIM2HRB) bug1.bim bug1.toy 0

bug1.bim: bug1.obj user_api.obj
	$(OBJ2BIM) @$(RULEFILE) out:bug1.bim map:bug1.map bug1.obj user_api.obj

bug2.toy: bug2.bim
	$(BIM2HRB) bug2.bim bug2.toy 0

bug2.bim: bug2.obj 
	$(OBJ2BIM) @$(RULEFILE) out:bug2.bim map:bug2.map bug2.obj 

bug3.toy: bug3.bim
	$(BIM2HRB) bug3.bim bug3.toy 0

bug3.bim: bug3.obj user_api.obj
	$(OBJ2BIM) @$(RULEFILE) out:bug3.bim map:bug3.map bug3.obj user_api.obj

hello4.toy: hello4.bim
	$(BIM2HRB) hello4.bim hello4.toy 0

hello4.bim: hello4.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:hello4.bim map:hello4.map \
		hello4.obj user_api.obj 

hello5.toy: hello5.bim
	$(BIM2HRB) hello5.bim hello5.toy 0

hello5.bim: hello5.obj 
	$(OBJ2BIM) @$(RULEFILE) out:hello5.bim stack:1k map:hello5.map hello5.obj

##########################################################
##########################################################
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
