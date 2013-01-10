BIN	= test
CC	= clang
CFLAG	= -g -O2 -Wall
RM	= rm 
OBJS	= main.o stack.o


$(BIN): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) *.o $(BIN)
