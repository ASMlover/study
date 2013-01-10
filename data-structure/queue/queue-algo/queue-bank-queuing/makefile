BIN	= test
CC	= clang
CFLAGS	= -g -O2 -Wall 
RM	= rm 
OBJS	= ../lib/queue.o\
				main.o bank_queuing.o


$(BIN): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) $(OBJS) $(BIN)
