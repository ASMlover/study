BIN	= test
CC	= clang
CFLAGS	= -g -O2 -Wall
RM	= rm 
OBJS	= ../lib/queue.o\
				main.o dancing_partner.o


$(BIN): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) $(OBJS) $(BIN)
