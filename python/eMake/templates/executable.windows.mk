# executable makefile for windows

OUTOBJ = obj
OUTBIN = bin
TARGET = $(OUTBIN)/{target}.exe
RM = del /s /f /q
CC = cl -c -nologo
MT = mt -nologo
LINK = link -nologo
CFLAGS = -GS -Zi -Fd$(OUTBIN)/vc.pdb{cflags}{preprocessor}{inc_dir}
LDFLAGS = -INCREMENTAL -DEBUG -PDB:$(TARGET).pdb -manifest -manifestfile:$(TARGET).manifest -manifestuac:no{ldflags}{link_dir}{link_libs}
OBJS = {objs}

all: $(TARGET)

rebuild: clean all

clean:
	$(RM) $(OUTBIN)\*.* $(OUTOBJ)\*.*

$(TARGET): $(OBJS)
	$(LINK) -out:$(TARGET) $(OBJS) $(LDFLAGS)
	$(MT) -manifest $(TARGET).manifest -outputresource:$(TARGET);1

{emake_objs}
