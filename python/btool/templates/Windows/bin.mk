# application Makefile

BIN = {bin}
RM = del
CC = cl -c -nologo
MT = mt -nologo
LINK = link -nologo
CFLAGS = -GS -Zi -Fd"vc.pdb" -EHsc{cflags}{preprocessor}{inc_dir}
LDFLAGS = -INCREMENTAL -DEBUG -PDB:$(BIN).pdb -manifest -manifestfile:$(BIN).manifest -manifestuac:no{ldflags}{lib_dir}{dep_libs}
OBJS = {objs}
SRCS = {srcs}

all: $(BIN)

rebuild: clean all

clean:
	$(RM) $(BIN) $(OBJS) *.pdb *.ilk *.manifest

$(BIN): $(OBJS)
	$(LINK) -out:$(BIN) $(OBJS) $(LDFLAGS)
	$(MT) -manifest $(BIN).manifest -outputresource:$(BIN);1

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)
