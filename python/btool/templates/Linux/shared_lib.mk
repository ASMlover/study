# shared library makefile

OUT = {out}.so
RM = rm
CC = g++
CFLAGS = -g -fPIC{cflags}{inc_dir}
LDFLAGS = -shared{ldflags}{lib_dir}{dep_libs}
OBJS = {objs}
SRCS = {srcs}

all: $(OUT)

rebuild: clean all

clean:
	$(RM) $(OUT) $(OBJS)

$(OUT): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): $(SRCS)
	$(CC) -o $*.o -c $(CFLAGS) $^
