LRUFILE = multitiersimulator.c
CC = gcc
LIBCACHE_FLAGS = -I/usr/local/include/ -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -L/usr/local/lib/ -llibCacheSim -lglib-2.0
EX_FLAGS = -lm -ldl

lru.o: $(LRUFILE)
	$(CC) $(LIBCACHE_FLAGS) $(EX_FLAGS) $(LRUFILE) -o $@