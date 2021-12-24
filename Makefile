SIMULATOR = multitiersimulator.c
CC = gcc
LIBCACHE_FLAGS = -I/usr/local/include/ -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -L/usr/local/lib/ -llibCacheSim -lglib-2.0
EX_FLAGS = -lm -ldl

simulator.o: $(SIMULATOR)
	$(CC) $(LIBCACHE_FLAGS) $(EX_FLAGS) $(SIMULATOR) -o $@

clean:
	rm -rf *.out *.o