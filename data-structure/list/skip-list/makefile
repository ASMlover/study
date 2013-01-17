BIN	= test
CC	= clang 
CFLAGS	= -g -O2 -D NDEBUG -Wall 
RM	= rm 
OBJS	= main.o skip_list.o 


$(BIN): $(OBJS) 
	$(CC) -o $@ $^

clean:
	$(RM) $(OBJS) $(BIN)
