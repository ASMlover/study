# static library makefile for linux

OUTOBJ = obj
OUTBIN = bin
TARGET = $(OUTBIN)/{target}.a
RM = rm
CC = g++
AR = ar
CFLAGS = -g{cflags}{inc_dir}
OBJS = {objs}

all: $(TARGET)

rebuild: clean all

clean:
	$(RM) $(OUTBIN)/*.* $(OUTOBJ)/*.*

$(TARGET): $(OBJS)
	$(AR) -cru $@ $^

{emake_objs}
