BIN	= test
CC	= clang
CFLAGS	= -g -O2 -Wall
RM 	= rm 
OBJS	= ../base/stack.o\
				main.o bracket_match.o


$(BIN): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) *.o ../base/*.o $(BIN)
