BIN	= lbbroker
CC	= cc 
RM	= rm 
CFLAGS	= -g -O2 -Wall -I../utils
LDFLAGS	= -lpthread -lzmq -L../utils/lib -lutils
OBJS	= main.o client.o server.o broker.o


$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) 

$(OBJS): %.o: %.c
	$(CC) -o $*.o -c $(CFLAGS) $^

clean:
	$(RM) $(BIN) $(OBJS)
