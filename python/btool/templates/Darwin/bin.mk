# application Makefile

BIN = {bin}
RM = rm
CC = clang++
CFLAGS = -g{cflags}{inc_dir}
LDFLAGS = -lc {lib_dir}{dep_libs}
OBJS = {objs}
SRCS = {srcs}

all: $(BIN)

rebuild: clean all

clean:
	$(RM) $(BIN) $(OBJS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): $(SRCS)
	$(CC) -o $*.o -c $(CFLAGS) $^
