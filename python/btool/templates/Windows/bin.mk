# application Makefile

OUT = {out}.exe
RM = del
CC = cl -c -nologo
MT = mt -nologo
LINK = link -nologo
CFLAGS = -GS -Zi -Fd"vc.pdb" -EHsc{cflags}{preprocessor}{inc_dir}
LDFLAGS = -INCREMENTAL -DEBUG -PDB:$(OUT).pdb -manifest -manifestfile:$(OUT).manifest\
	-manifestuac:no{ldflags}{lib_dir}{dep_libs}
OBJS = {objs}
SRCS = {srcs}

all: $(OUT)

rebuild: clean all

clean:
	$(RM) $(OUT) $(OBJS) *.pdb *.ilk *.manifest

$(OUT): $(OBJS)
	$(LINK) -out:$(OUT) $(OBJS) $(LDFLAGS)
	$(MT) -manifest $(OUT).manifest -outputresource:$(OUT);1

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)
