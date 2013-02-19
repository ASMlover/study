REP_BIN	= rep
REQ_BIN	= req
CC	= gcc
RM	= rm 
CFLAGS   = -g -O2 -Wall
LDFLAGS  = -lzmq
REP_OBJS = rep.o
REQ_OBJS = req.o


all: $(REP_BIN) $(REQ_BIN)

$(REP_BIN): $(REP_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(REQ_BIN): $(REQ_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(REP_OBJS): %.o: %.c
	$(CC) -o $*.o -c $(CFLAGS) $^

$(REQ_OBJS): %.o: %.c
	$(CC) -o $*.o -c $(CFLAGS) $^

clean:
	$(RM) $(REP_OBJS) $(REP_BIN) $(REQ_OBJS) $(REQ_BIN)
