$(OUTOBJ)/{mk_obj}: {mk_src}
	$(CC) -Fo:$(OUTOBJ)/{mk_obj} $(CFLAGS) {mk_src}

