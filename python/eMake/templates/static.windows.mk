# static library makefile for windows

OUTOBJ = obj
OUTBIN = bin
TARGET = $(OUTBIN)/{target}.lib
RM = del /s /f /q
CC = cl -c -nologo
AR = lib -nologo
CFLAGS = -GS -Zi -Fd$(OUTBIN)/vc.pdb{cflags}{preprocessor}{inc_dir}
OBJS = {objs}

all: $(TARGET)

rebuild: clean all

clean:
	$(RM) $(OUTBIN)\*.* $(OUTOBJ)\*.*

$(TARGET): $(OBJS)
	$(AR) -out:$(TARGET) $(OBJS)

{emake_objs}
