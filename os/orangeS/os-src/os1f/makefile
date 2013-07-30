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

BOOT_BIN = boot.bin 
BOOT_SRC = boot.s
LDR_BIN	= loader.bin 
LDR_SRC	= loader.s 
KERNEL_BIN = kernel.bin 
KERNEL_OBJS = kernel.o string.o start.o kliba.o
RM	= rm 
NASM	= nasm 
CC	= gcc
LD	= ld
DD	= dd 
IMG	= a.img 
FLOPPY	= /mnt/floppy/
BOCHS	= bochs 
BOCHSCFG = bochsrc 



all: $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN) 

rebuild: clean all

install:
	$(DD) if=$(BOOT_BIN) of=$(IMG) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(IMG) $(FLOPPY)
	sudo cp $(LDR_BIN) $(FLOPPY) -v 
	sudo cp $(KERNEL_BIN) $(FLOPPY) -v
	sudo umount $(FLOPPY)

run:
	$(BOCHS) -f $(BOCHSCFG)

clean:
	$(RM) $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN) $(IMG) *.o *.log





$(BOOT_BIN): $(BOOT_SRC)
	$(NASM) -o $@ $^ 

$(LDR_BIN): $(LDR_SRC)
	$(NASM) -o $@ $^ 

$(KERNEL_BIN): $(KERNEL_OBJS)
	$(LD) -s -Ttext 0x30400 -o $@ $^

kernel.o: kernel.s
	$(NASM) -f elf -o $@ $^

string.o: string.s
	$(NASM) -f elf -o $@ $^ 

kliba.o: kliba.s
	$(NASM) -f elf -o $@ $^

start.o: start.c
	$(CC) -c -fno-builtin -o $@ $^
