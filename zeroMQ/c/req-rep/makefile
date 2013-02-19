REP_BIN	= rep
REQ_BIN	= req
CC	= gcc
RM	= rm 
CFLAGS   = -g -O2 -Wall
LDFLAGS  = -lzmq
REP_OBJS = rep.o
REQ_OBJS = 


$(REP_BIN): $(REP_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(REP_OBJS): %.o: %.c
	$(CC) -o $*.o -c $(CFLAGS) $^

clean:
	$(RM) $(REP_OBJS) $(REP_BIN)
