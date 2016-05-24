# static library makefile

OUT = {out}.a
RM = rm
CC = g++
AR = ar
CFLAGS = -g{cflags}{inc_dir}
OBJS = {objs}
SRCS = {srcs}

all: $(OUT)

rebuild: clean all

clean:
	$(RM) $(OUT) $(OBJS)

$(OUT): $(OBJS)
	$(AR) -cru $@ $^

$(OBJS): $(SRCS)
	$(CC) -o $*.o -c $(CFLAGS) $^
