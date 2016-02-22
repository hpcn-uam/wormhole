export CC=gcc
export CXX=g++
export FLAGS=-I include/ -Wall -Werror -g -lpthread -O0
export CFLAGS=$(FLAGS) -std=gnu99 -fPIC
export CXXFLAGS=$(FLAGS) -std=gnu++11
export LDFLAGS=-fPIC -ldl -lpthread

INCLUDES := $(wildcard include/*.h include/*.hpp)
SRCS := $(wildcard src/*.c src/*.cpp src/examples/*.c src/examples/*.cpp)


all: Dependencies einstein libs Examples doc/html

einstein: obj/einstein.o

Examples: bin/testEinstein bin/testWorm bin/testLisp bin/testWorm.tgz bin/testLisp.tgz bin/testSendAsync bin/testRecvAsync

bin/testWorm.tgz: bin/testWorm lib/libworm.so src/run.sh
	mkdir -p bin/tmp/testWorm/lib
	cp bin/testWorm bin/tmp/testWorm
	cp lib/libworm.so bin/tmp/testWorm/lib
	cp src/run.sh bin/tmp/testWorm
	cd bin/tmp;	tar -czf testWorm.tgz testWorm
	mv bin/tmp/testWorm.tgz bin/testWorm.tgz
	rm -rf bin/tmp

bin/testLisp.tgz: bin/testLisp lib/libworm.so src/examples/lisprun.sh
	mkdir -p bin/tmp/testLisp/lib
	cp bin/testLisp bin/tmp/testLisp
	cp lib/libworm.so bin/tmp/testLisp/lib
	cp src/examples/lisprun.sh bin/tmp/testLisp/run.sh
	cd bin/tmp;	tar -czf testLisp.tgz testLisp
	mv bin/tmp/testLisp.tgz bin/testLisp.tgz
	rm -rf bin/tmp

bin/testEinstein: src/examples/testEinstein.cpp obj/einstein.o obj/common.o
	$(CXX) $(CXXFLAGS) -o $@ $^

bin/testWorm: src/examples/testWorm.c obj/common.o
	$(CC) $(CFLAGS) -Llib -lworm -o $@ $^

bin/testLisp: src/examples/testLisp.c obj/common.o
	$(CC) $(CFLAGS) $(LDFLAGS) -Llib -lworm -o $@ $^

bin/testSendAsync: src/examples/testSendAsync.c obj/common.o
	$(CC) $(CFLAGS) -o $@ $^

bin/testRecvAsync: src/examples/testRecvAsync.c obj/common.o
	$(CC) $(CFLAGS) -o $@ $^


lib/libworm.so: obj/worm.o obj/common.o obj/structures.h.o
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

Dependencies: obj lib bin

obj:
	mkdir -p obj

lib:
	mkdir -p lib

doc/html: $(INCLUDES) $(SRCS)
	doxygen > /dev/null

bin:
	mkdir -p bin

libs: lib lib/libworm.so

buildTools:
	$(MAKE) -C tools

obj/%.o: src/%.cpp $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/%.o: src/%.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf obj lib bin


#Custom Data .o
obj/structures.h.o: $(INCLUDES)
	./tools/parseFile.bash > obj/structures.h
	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 obj/structures.h $@
