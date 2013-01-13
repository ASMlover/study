BIN	= test
CC	= gcc
CFLAGS	= -g -O2 -Wall 
RM	= rm 
OBJS	= main.o list.o 


$(BIN): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) $(OBJS) $(BIN)
