BIN	= test
CC	= clang
CFLAGS	= -g -O2 -Wall
RM	= rm 
OBJS	= ../base/stack.o\
				main.o conversion.o


$(BIN): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) $(OBJS) $(BIN)
