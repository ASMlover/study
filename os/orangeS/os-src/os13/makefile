
BIN	= boot.bin 
SRC	= boot.s
RM	= rm 
NASM	= nasm 
DD	= dd 
IMG	= a.img 
BOCHS	= bochs 
BOCHSCFG = bochsrc

$(BIN): $(SRC)
	$(NASM) -o $@ $^

install:
	$(DD) if=$(BIN) of=$(IMG) bs=512 count=1 conv=notrunc

run:
	$(BOCHS) -f $(BOCHSCFG)

clean:
	$(RM) $(BIN) *.img *.log
