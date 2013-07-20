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
LDR_BIN	= loader.com 
LDR_SRC	= loader.s
RM	= rm 
NASM	= nasm 
DD	= dd 
IMG	= a.img 
PMIMG	= pm.img
BOCHS	= bochs 
BOCHSCFG = bochsrc



all: $(BOOT_BIN) $(LDR_BIN)

$(BOOT_BIN): $(BOOT_SRC)
	$(NASM) -o $@ $^ 

$(LDR_BIN): $(LDR_SRC)
	$(NASM) -o $@ $^

install:
	$(DD) if=$(BOOT_BIN) of=$(IMG) bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy/ 
	sudo cp $(LDR_BIN) /mnt/floppy/ -v 
	sudo umount /mnt/floppy/ 

install_loader:
	sudo mount -o loop pm.img /mnt/floppy/ 
	sudo cp $(LDR_BIN) /mnt/floppy/ -v 
	sudo umount /mnt/floppy/ 

run:
	$(BOCHS) -f $(BOCHSCFG)

clean:
	$(RM) $(BOOT_BIN) $(LDR_BIN) $(IMG) $(PMIMG) *.log
