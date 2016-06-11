# executable makefile for darwin

OUTOBJ = obj
OUTBIN = bin
TARGET = $(OUTBIN)/{target}
RM = rm -rf
CC = g++
CFLAGS = -g{cflags} {inc_dir}
LDFLAGS = -lc{ldflags}{link_dir}{link_libs}
OBJS = {objs}

all: $(TARGET)

rebuild: clean all

clean:
	$(RM) $(OUTBIN)/*.* $(OUTOBJ)/*.*

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

{emake_objs}
