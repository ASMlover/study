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


ENTRYPOINT = 0x30400

BOOT_BIN = ./boot/boot.bin 
LDR_BIN	= ./boot/loader.bin 
SBFLAGS	= -I./boot/include/
SKFLAGS	= -I./include/ -f elf 
CFLAGS	= -I ./include/ -c -fno-builtin 
LDFLAGS	= -s -Ttext $(ENTRYPOINT)
KERNEL_BIN = ./kernel/kernel.bin 
KERNEL_OBJS = ./kernel/kernel.o \
	./kernel/start.o \
	./lib/string.o \
	./lib/kliba.o \
	./kernel/i8259.o
RM	= rm 
NASM	= nasm 
CC	= gcc
LD	= ld
DD	= dd 
IMG	= a.img 
FLOPPY	= /mnt/floppy/
QEMU	= qemu 



all: $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN) 

rebuild: clean all

install:
	$(DD) if=$(BOOT_BIN) of=$(IMG) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(IMG) $(FLOPPY)
	sudo cp $(LDR_BIN) $(FLOPPY) -v 
	sudo cp $(KERNEL_BIN) $(FLOPPY) -v
	sudo umount $(FLOPPY)

run:
	$(QEMU) -fda $(IMG)

clean:
	$(RM) $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN) $(IMG) $(KERNEL_OBJS) *.log





$(BOOT_BIN): ./boot/boot.s
	$(NASM) $(SBFLAGS) -o $@ $^ 

$(LDR_BIN): ./boot/loader.s
	$(NASM) $(SBFLAGS) -o $@ $^ 

$(KERNEL_BIN): $(KERNEL_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

./kernel/kernel.o: ./kernel/kernel.s
	$(NASM) $(SKFLAGS) -o $@ $^

./lib/string.o: ./lib/string.s
	$(NASM) $(SKFLAGS) -o $@ $^ 

./lib/kliba.o: ./lib/kliba.s
	$(NASM) $(SKFLAGS) -o $@ $^

./kernel/start.o: ./kernel/start.c
	$(CC) $(CFLAGS) -o $@ $^ 

./kernel/i8259.o: ./kernel/i8259.c
	$(CC) $(CFLAGS) -o $@ $^
