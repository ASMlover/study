BIN	= test
CC	= clang
CFLAGS	= -g -O2 -Wall
RM	= rm 
OBJS	= main.o stack.o calc.o

$(BIN): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) *.o $(BIN)
