# application Makefile

OUT = {out}
RM = rm
CC = g++
CFLAGS = -g{cflags}{inc_dir}
LDFLAGS = {lib_dir}{dep_libs}
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
