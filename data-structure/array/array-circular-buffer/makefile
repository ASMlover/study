BIN	= test
CC	= clang 
CFLAGS	= -g -O2 -Wall 
RM	= rm 
OBJS	= main.o circular_buffer.o 

$(BIN): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) $(OBJS) $(BIN)
