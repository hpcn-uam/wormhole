CC=gcc
CXX=g++
FLAGS=-I include/ -Wall -Werror
CFLAGS=$(FLAGS) -std=c99 -fPIC
CXXFLAGS=$(FLAGS) -std=c++11
LDFLAGS=-fPIC

INCLUDES := $(wildcard include/*.h include/*.hpp)


all: einstein libs

einstein: obj/einstein.o

testEinstein: src/examples/testEinstein.cpp obj/einstein.o obj/common.o
	$(CXX) $(CXXFLAGS) -o $@ $^

testWorm: src/examples/testWorm.c obj/common.o
	$(CC) $(CFLAGS) -Llib -lworm -o $@ $^

lib/libworm.so: obj/worm.o obj/common.o
	$(CC) $(CFLAGS) -shared -o $@ $^  $(LDFLAGS)

obj:
	mkdir -p obj

libs: lib lib/libworm.so

lib:
	mkdir -p lib

obj/%.o: src/%.cpp obj $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/%.o: src/%.c obj $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf obj lib
