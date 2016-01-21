CC=gcc
CFLAGS=-I include/ -Wall -Werror

worm: build/worm.o build/common.o

build/worm.o: src/worm/worm.c
	$(CC) $(CFLAGS) -c $< -o $@

build/common.o: src/common.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm build/*