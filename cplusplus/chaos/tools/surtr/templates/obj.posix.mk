{build_obj}: {build_src}
	$(CC) -o $@ -c $(CFLAGS) $^
