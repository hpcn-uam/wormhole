#VPATH
export JAVAPATH = src/langApis/Java/
export VPATH = $(JAVAPATH)
.SUFFIXES: .java .class

#Common
export TMPDIR=/tmp/

#C/C++
export CC=gcc
export CXX=g++
export FLAGS=-fPIC -I include/ -Wall -g -lpthread -pthread -O3 -Werror
export CFLAGS=$(FLAGS) -std=gnu11
export CXXFLAGS=$(FLAGS) -std=gnu++11
export LDFLAGS=-fPIC -ldl -lpthread -lstdc++

#Java
export JFLAGS = -g
export JC = javac
export JCFLAGS=-I $(JAVA_HOME)/include/ -I $(JAVA_HOME)/include/linux/
export JLDFLAGS=$(LDFLAGS) -Llib -lworm
export JAVALIBSRC := $(wildcard src/langApis/Java/es/hpcn/wormhole/*.java src/langApis/Java/es/hpcn/wormhole/test/*.java)
export STRMLIBSRC := $(wildcard src/langApis/Java/backtype/storm/*.java src/langApis/Java/backtype/storm/spout/*.java src/langApis/Java/backtype/storm/task/*.java src/langApis/Java/backtype/storm/topology/*.java src/langApis/Java/backtype/storm/topology/base/*.java src/langApis/Java/backtype/storm/tuple/*.java src/langApis/Java/backtype/storm/utils/*.java )
export JAVAFILES  := $(JAVALIBSRC) $(STRMLIBSRC)
export CLASSFILES :=  $(patsubst $(JAVAPATH)%,%,$(JAVAFILES:.java=.class))

export INCLUDES := $(wildcard include/*.h include/*.hpp)
export SRCS := $(wildcard src/*.c src/*.cpp src/examples/*.c src/examples/*.cpp)

all: Dependencies einstein libs langLibs Examples doc/html

langLibs: javaLibs

Examples: bin/testEinstein bin/testWorm bin/testLisp bin/testWorm.tgz bin/testLisp.tgz bin/testBW.tgz bin/testSendAsync bin/testRecvAsync
Jexamples: bin/testJBW.tgz

#Tars
bin/testWorm.tgz: bin/testWorm lib/libworm.so src/run.sh
	mkdir -p $(TMPDIR)/testWorm/lib
	cp bin/testWorm $(TMPDIR)/testWorm
	cp lib/libworm.so $(TMPDIR)/testWorm/lib
	cp src/run.sh $(TMPDIR)/testWorm
	cd $(TMPDIR);	tar -czf testWorm.tgz testWorm
	mv $(TMPDIR)/testWorm.tgz bin/testWorm.tgz
	rm -rf $(TMPDIR)/testWorm

bin/testLisp.tgz: bin/testLisp lib/libworm.so src/examples/lisprun.sh
	mkdir -p $(TMPDIR)/testLisp/lib
	cp bin/testLisp $(TMPDIR)/testLisp
	cp lib/libworm.so $(TMPDIR)/testLisp/lib
	cp src/examples/lisprun.sh $(TMPDIR)/testLisp/run.sh
	cd $(TMPDIR);	tar -czf testLisp.tgz testLisp
	mv $(TMPDIR)/testLisp.tgz bin/testLisp.tgz
	rm -rf $(TMPDIR)/testLisp
	
bin/testBW.tgz: bin/testBW lib/libworm.so src/examples/bwrun.sh
	mkdir -p $(TMPDIR)/testBW/lib
	cp bin/testBW $(TMPDIR)/testBW
	cp lib/libworm.so $(TMPDIR)/testBW/lib
	cp src/examples/bwrun.sh $(TMPDIR)/testBW/run.sh
	cd $(TMPDIR);	tar -czf testBW.tgz testBW
	mv $(TMPDIR)/testBW.tgz bin/testBW.tgz
	rm -rf $(TMPDIR)/testBW
	
bin/testJBW.tgz: lib/libworm.so lib/libjavaworm.so lib/libjavaworm.jar src/examples/jbwrun.sh
	mkdir -p $(TMPDIR)/testJBW/lib
	cp lib/libjavaworm.* $(TMPDIR)/testJBW/lib #only for java
	cp lib/libworm.so $(TMPDIR)/testJBW/lib
	cp src/examples/jbwrun.sh $(TMPDIR)/testJBW/run.sh
	cd $(TMPDIR);	tar -czf testJBW.tgz testJBW
	mv $(TMPDIR)/testJBW.tgz bin/testJBW.tgz
	rm -rf $(TMPDIR)/testJBW
	
bin/testSTBW.tgz: lib/libworm.so lib/libjavaworm.so lib/libjavaworm.jar src/examples/stormrun.sh
	mkdir -p $(TMPDIR)/testJBW/lib
	cp lib/libjavaworm.* $(TMPDIR)/testJBW/lib #only for java
	cp lib/libworm.so $(TMPDIR)/testJBW/lib
	cp src/examples/stormrun.sh $(TMPDIR)/testJBW/run.sh
	cd $(TMPDIR); tar -czf testJBW.tgz testJBW
	mv $(TMPDIR)/testJBW.tgz bin/testJBW.tgz
	rm -rf $(TMPDIR)/testJBW

#Examples
bin/testEinstein: src/examples/testEinstein.cpp obj/einstein.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -Llib -lworm -o $@ $^

bin/testWorm: src/examples/testWorm.c
	$(CC) $(CFLAGS) $(LDFLAGS) -Llib -lworm -o $@ $^

bin/testLisp: src/examples/testLisp.c
	$(CC) $(CFLAGS) $(LDFLAGS) -Llib -lworm -o $@ $^
	
bin/testBW: src/examples/testBW.c
	$(CC) $(CFLAGS) $(LDFLAGS) -Llib -lworm -o $@ $^

bin/testSendAsync: src/examples/testSendAsync.c obj/common.o
	$(CC) $(CFLAGS) -o $@ $^

bin/testRecvAsync: src/examples/testRecvAsync.c obj/common.o
	$(CC) $(CFLAGS) -o $@ $^


lib/libworm.so: obj/worm.o obj/common.o obj/structures.h.o obj/einstein.o
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

#JAVALibs
lib/libjavaworm.so: lib/libworm.so $(JAVAPATH)es_hpcn_wormhole_Worm.h $(JAVAPATH)es_hpcn_wormhole_Worm.c $(JAVAPATH)es_hpcn_wormhole_Einstein.h $(JAVAPATH)es_hpcn_wormhole_Einstein.cpp
	$(CC)  $(JCFLAGS) $(CFLAGS)   $(JLDFLAGS) -c -o obj/es_hpcn_wormhole_Worm.o $(JAVAPATH)es_hpcn_wormhole_Worm.c
	$(CXX) $(JCFLAGS) $(CXXFLAGS) $(JLDFLAGS) -c -o obj/es_hpcn_wormhole_Einstein.o $(JAVAPATH)es_hpcn_wormhole_Einstein.cpp
	$(CC)  $(JCFLAGS) $(CFLAGS)   $(JLDFLAGS) -shared -o $@ obj/es_hpcn_wormhole_Worm.o obj/es_hpcn_wormhole_Einstein.o

lib/libjavaworm.jar: $(CLASSFILES)
	cd $(JAVAPATH); jar cf libjavaworm.jar $(CLASSFILES)
	mv $(JAVAPATH)libjavaworm.jar lib/libjavaworm.jar

$(JAVAPATH)es_hpcn_wormhole_Worm.h: $(JAVAPATH)es/hpcn/wormhole/Worm.java
	cd $(JAVAPATH) ; rm -f es_hpcn_wormhole_Worm.h ; javah es.hpcn.wormhole.Worm
	
$(JAVAPATH)es_hpcn_wormhole_Einstein.h: $(JAVAPATH)es/hpcn/wormhole/Einstein.java
	cd $(JAVAPATH) ; rm -f es_hpcn_wormhole_Einstein.h ; javah es.hpcn.wormhole.Einstein

.java.class:
	cd $(JAVAPATH); $(JC) $(JFLAGS) $*.java
	
#Common
Dependencies: obj lib bin

einstein: obj/einstein.o

obj:
	mkdir -p obj

lib:
	mkdir -p lib

doc/html: $(INCLUDES) $(SRCS)
	doxygen > /dev/null

bin:
	mkdir -p bin

libs: lib lib/libworm.so

javaLibs: lib/libjavaworm.so lib/libjavaworm.jar Jexamples

buildTools:
	$(MAKE) -C tools

obj/%.o: src/%.cpp $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/%.o: src/%.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf obj lib bin
	cd $(JAVAPATH); rm -rf $(CLASSFILES)


#Custom Data .o
obj/structures.h.o: $(INCLUDES)
	./tools/parseFile.bash > obj/structures.h
	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 obj/structures.h $@
