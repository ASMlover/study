PUB_BIN	= pub 
SUB_BIN	= sub 
CC	= gcc 
RM	= rm 
CFLAGS  = -g -O2 -Wall 
LDFLAGS = -lzmq
PUB_OBJS = pub.o
SUB_OBJS = sub.o


all: $(PUB_BIN) $(SUB_BIN)

clean: 
	$(RM) $(PUB_OBJS) $(PUB_BIN) $(SUB_OBJS) $(SUB_BIN)

$(PUB_BIN): $(PUB_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(SUB_BIN): $(SUB_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(PUB_OBJS): %.o: %.c
	$(CC) -o $*.o -c $(CFLAGS) $^

$(SUB_OBJS): %.o: %.c
	$(CC) -o $*.o -c $(CFLAGS) $^
