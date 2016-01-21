CC=gcc
CXX=g++
COMMONFLAGS=-I include/ -Wall -Werror
CFLAGS=$(COMMONFLAGS) -std=c99
CXXFLAGS=$(COMMONFLAGS) -std=c++11

all: einstein worm

einstein: build/einstein.o

build/einstein.o: src/einstein/einstein.cpp include/einstein.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

worm: build/worm.o build/common.o

build/worm.o: src/worm/worm.c include/worm.h include/worm_private.h include/common.h
	$(CC) $(CFLAGS) -c $< -o $@

build/common.o: src/common.c include/common.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm build/*