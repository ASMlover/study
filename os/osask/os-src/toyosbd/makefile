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


$(IMG): $(OBJ_BIN) $(OBJ_SYS) $(APP) $(APP2) a.toy hello3.toy hello4.toy \
	hello5.toy win.toy win2.toy win3.toy star.toy stars.toy stars2.toy \
	lines.toy walk.toy noodle.toy beepdown.toy color.toy
	$(EDIMG) imgin:$(FDIMG0AT) wbinimg src:$(OBJ_BIN) len:512 from:0 to:0\
		copy from:$(OBJ_SYS) to:@: \
		copy from:toyos.s to:@: \
		copy from:makefile to:@: \
		copy from:cat.txt to:@: \
		copy from:$(APP) to:@: \
		copy from:$(APP2) to:@: \
		copy from:a.toy to:@: \
		copy from:hello3.toy to:@: \
		copy from:hello4.toy to:@: \
		copy from:hello5.toy to:@: \
		copy from:win.toy to:@: \
		copy from:win2.toy to:@: \
		copy from:win3.toy to:@: \
		copy from:star.toy to:@: \
		copy from:stars.toy to:@: \
		copy from:stars2.toy to:@: \
		copy from:lines.toy to:@: \
		copy from:walk.toy to:@: \
		copy from:noodle.toy to:@: \
		copy from:beepdown.toy to:@: \
		copy from:color.toy to:@: \
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

hello4.toy: hello4.bim
	$(BIM2HRB) hello4.bim hello4.toy 0

hello4.bim: hello4.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:hello4.bim map:hello4.map \
		hello4.obj user_api.obj 

hello5.toy: hello5.bim
	$(BIM2HRB) hello5.bim hello5.toy 0

hello5.bim: hello5.obj 
	$(OBJ2BIM) @$(RULEFILE) out:hello5.bim stack:1k map:hello5.map hello5.obj 

win.toy: win.bim
	$(BIM2HRB) win.bim win.toy 0

win.bim: win.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:win.bim stack:1k map:win.map \
		win.obj user_api.obj

win2.toy: win2.bim
	$(BIM2HRB) win2.bim win2.toy 0

win2.bim: win2.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:win2.bim stack:1k map:win2.map \
		win2.obj user_api.obj

win3.toy: win3.bim
	$(BIM2HRB) win3.bim win3.toy 40k

win3.bim: win3.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:win3.bim stack:1k map:win3.map \
		win3.obj user_api.obj 

star.toy: star.bim
	$(BIM2HRB) star.bim star.toy 47k

star.bim: star.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:star.bim stack:1k map:star.map \
		star.obj user_api.obj 

stars.toy: stars.bim
	$(BIM2HRB) stars.bim stars.toy 47k

stars.bim: stars.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:stars.bim stack:1k map:stars.map \
		stars.obj user_api.obj 

stars2.toy: stars2.bim
	$(BIM2HRB) stars2.bim stars2.toy 47k

stars2.bim: stars2.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:stars2.bim stack:1k map:stars2.map \
		stars2.obj user_api.obj 

lines.toy: lines.bim
	$(BIM2HRB) lines.bim lines.toy 48k

lines.bim: lines.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:lines.bim stack:1k map:lines.map \
		lines.obj user_api.obj 

walk.toy: walk.bim
	$(BIM2HRB) walk.bim walk.toy 48k

walk.bim: walk.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:walk.bim stack:1k map:walk.map \
		walk.obj user_api.obj 

noodle.toy: noodle.bim
	$(BIM2HRB) noodle.bim noodle.toy 40k

noodle.bim: noodle.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:noodle.bim stack:1k map:noodle.map \
		noodle.obj user_api.obj 

beepdown.toy: beepdown.bim
	$(BIM2HRB) beepdown.bim beepdown.toy 40k

beepdown.bim: beepdown.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:beepdown.bim stack:1k map:beepdown.map \
		beepdown.obj user_api.obj 

color.toy: color.bim
	$(BIM2HRB) color.bim color.toy 56k

color.bim: color.obj user_api.obj 
	$(OBJ2BIM) @$(RULEFILE) out:color.bim stack:1k map:color.map \
		color.obj user_api.obj 

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
