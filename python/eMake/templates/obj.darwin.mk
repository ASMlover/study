{mk_obj}: {mk_src}
	$(CC) -o {mk_obj} -c $(CFLAGS) -std=c++11 {mk_src}
