BIN	= test
CC	= clang
CFLAGS	= -g -D NDEBUG -O2 -Wall 
RM	= rm 
OBJS	= main.o deque.o


$(BIN): $(OBJS)
	$(CC) -o $@ $^

re: $(BIN) 

clean:
	$(RM) $(OBJS) $(BIN)
