# static library makefile for darwin

OUTOBJ = obj
OUTBIN = bin
TARGET = $(OUTBIN)/{target}.a
RM = rm
CC = clang++
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
