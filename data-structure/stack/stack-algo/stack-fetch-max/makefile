BIN	= test
CC	= clang
CFLAGS	= -g -O2 -Wall
OBJS	= main.o stack.o
RM	= rm 

$(BIN): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) *.o $(BIN)
