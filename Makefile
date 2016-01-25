export CC=gcc
export CXX=g++
export FLAGS=-I include/ -Wall -Werror -g
export CFLAGS=$(FLAGS) -std=c99 -fPIC
export CXXFLAGS=$(FLAGS) -std=c++11
export LDFLAGS=-fPIC

INCLUDES := $(wildcard include/*.h include/*.hpp)


all: einstein libs

einstein: obj/einstein.o

testEinstein: src/examples/testEinstein.cpp obj/einstein.o obj/common.o
	$(CXX) $(CXXFLAGS) -o $@ $^

testWorm: src/examples/testWorm.c obj/common.o
	$(CC) $(CFLAGS) -Llib -lworm -o $@ $^

lib/libworm.so: obj/worm.o obj/common.o obj/structures.h.o
	$(CC) $(CFLAGS) -shared -o $@ $^  $(LDFLAGS)

obj:
	mkdir -p obj

libs: lib lib/libworm.so

lib:
	mkdir -p lib

buildTools:
	$(MAKE) -C tools

obj/%.o: src/%.cpp obj $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/%.o: src/%.c obj $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf obj lib


#Custom Data .o
obj/structures.h.o: $(INCLUDES)
	./tools/parseFile.bash > obj/structures.h
	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 obj/structures.h $@
