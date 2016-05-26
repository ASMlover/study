# shared library Makefile

OUT = {out}.so
RM = rm
CC = clang++
CFLAGS = -g -fPIC{cflags}{inc_dir}
LDFLAGS = -shared -lc{ldflags}{lib_dir}{dep_libs}
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
