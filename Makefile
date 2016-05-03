#VPATH
export JAVAPATH = src/langApis/Java/
export VPATH = $(JAVAPATH)
.SUFFIXES: .java .class
.PRECIOUS: certs/prv/%.csr certs/prv/%.key.pem
.SECONDARY: certs/prv/%.csr certs/prv/%.key.pem
.NOTPARALLEL: all

#Common
export TMPDIR=/tmp/

#C/C++
export CC=gcc
export CXX=g++
export FLAGS=-fPIC -I include/ -Wall -Wextra -g -lpthread -pthread -O3 -Werror
export SSLCFLAGS= -I dependencies/compiled/libressl/usr/local/include
export CFLAGS=$(FLAGS) $(SSLCFLAGS) -std=gnu11
export CXXFLAGS=$(FLAGS) $(SSLCFLAGS) -std=gnu++11
#export SSLLDFLAGS=-lssl -lcrypto
#export SSLLDFLAGS= -Ldependencies/libressl/compiled/usr/local/lib/ -static -lssl -lcrypto #-Wl,-Bdynamic
export SSLLDFLAGS= dependencies/compiled/libressl/usr/local/lib/libtls.a dependencies/compiled/libressl/usr/local/lib/libssl.a dependencies/compiled/libressl/usr/local/lib/libcrypto.a

export LDFLAGS=-fPIC -ldl -lpthread -lstdc++

#Java
export JFLAGS = -g
export JC = javac
export JCFLAGS=-I $(JAVA_HOME)/include/ -I $(JAVA_HOME)/include/linux/
export JLDFLAGS=$(LDFLAGS) -Llib -lworm
export JAVALIBSRC := $(wildcard src/langApis/Java/es/hpcn/wormhole/*.java src/langApis/Java/es/hpcn/wormhole/test/*.java)
export STRMLIBSRC := $(wildcard src/langApis/Java/backtype/storm/*.java src/langApis/Java/backtype/storm/spout/*.java src/langApis/Java/backtype/storm/task/*.java src/langApis/Java/backtype/storm/topology/*.java src/langApis/Java/backtype/storm/topology/base/*.java src/langApis/Java/backtype/storm/tuple/*.java src/langApis/Java/backtype/storm/utils/*.java src/langApis/Java/backtype/storm/generated/*.java src/langApis/Java/backtype/storm/wh/tests/*.java)
export JAVAFILES  := $(JAVALIBSRC) $(STRMLIBSRC)
export CLASSFILES :=  $(patsubst $(JAVAPATH)%,%,$(JAVAFILES:.java=.class))

export INCLUDES := $(wildcard include/*.h include/*.hpp)
export SRCS := $(wildcard src/*.c src/*.cpp src/examples/*.c src/examples/*.cpp)

all: Dependencies libs langLibs bin/einstein Examples # doc/html

Docs: doc/html

langLibs: javaLibs

Examples: bin/testWorm bin/testLisp bin/testWorm.tgz bin/testLisp.tgz bin/testBW.tgz bin/testSendAsync bin/testRecvAsync bin/testSendSSL bin/testRecvSSL bin/testSendAsyncSSL bin/testRecvAsyncSSL
Jexamples: bin/testJBW.tgz bin/javaTest.tgz

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
	
bin/testBW.tgz: bin/testBW lib/libworm.so src/examples/bwrun.sh | SSL
	mkdir -p $(TMPDIR)/testBW/lib
	cp bin/testBW $(TMPDIR)/testBW
	cp -r certs $(TMPDIR)/testBW
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
	
bin/javaTest.tgz: lib/libworm.so lib/libjavaworm.so lib/libjavaworm.jar src/examples/javarun.sh
	mkdir -p $(TMPDIR)/javaTest/lib
	cp lib/libjavaworm.* $(TMPDIR)/javaTest/lib #only for java
	cp lib/libworm.so $(TMPDIR)/javaTest/lib
	cp src/examples/javarun.sh $(TMPDIR)/javaTest/run.sh
	cd $(TMPDIR); tar -czf javaTest.tgz javaTest
	mv $(TMPDIR)/javaTest.tgz bin/javaTest.tgz
	rm -rf $(TMPDIR)/javaTest
	
bin/nlp.tgz: lib/libworm.so lib/libjavaworm.so lib/libjavaworm.jar src/examples/javarun.sh dependencies/compiled/data/data.txt
	mkdir -p $(TMPDIR)/nlp/lib
	cp lib/libjavaworm.* $(TMPDIR)/nlp/lib #only for java
	cp lib/libworm.so $(TMPDIR)/nlp/lib
	cp src/examples/javarun.sh $(TMPDIR)/nlp/run.sh
	cp -r dependencies/compiled/nlp/* $(TMPDIR)/nlp/lib/.
	cp -r dependencies/compiled/data/data.txt $(TMPDIR)/nlp/
	cd $(TMPDIR); tar -czf nlp.tgz nlp
	mv $(TMPDIR)/nlp.tgz bin/nlp.tgz
	rm -rf $(TMPDIR)/nlp

#Examples
bin/testWorm: src/examples/testWorm.c
	$(CC) $(CFLAGS) -o $@ $^ -Llib -lworm $(LDFLAGS)

bin/testLisp: src/examples/testLisp.c
	$(CC) $(CFLAGS) -o $@ $^ -Llib -lworm $(LDFLAGS)
	
bin/testBW: src/examples/testBW.c
	$(CC) $(CFLAGS) -o $@ $^ -Llib -lworm $(LDFLAGS)

bin/testSendAsync: src/examples/testSendAsync.c obj/common.o
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS) $(SSLLDFLAGS) 

bin/testRecvAsync: src/examples/testRecvAsync.c obj/common.o
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS) $(SSLLDFLAGS) 
	
bin/testSendSSL: src/examples/testSendSSL.c obj/common.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(SSLLDFLAGS) $(SSLLDFLAGS)

bin/testRecvSSL: src/examples/testRecvSSL.c obj/common.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(SSLLDFLAGS) $(SSLLDFLAGS)

bin/testSendAsyncSSL: src/examples/testSendAsyncSSL.c obj/common.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(SSLLDFLAGS) $(SSLLDFLAGS)

bin/testRecvAsyncSSL: src/examples/testRecvAsyncSSL.c obj/common.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(SSLLDFLAGS) $(SSLLDFLAGS)

lib/libworm.so: obj/worm.o obj/common.o obj/structures.h.o obj/einstein.o
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^ $(SSLLDFLAGS)

#JAVALibs
lib/libjavaworm.so: lib/libworm.so $(JAVAPATH)es_hpcn_wormhole_Worm.h $(JAVAPATH)es_hpcn_wormhole_Worm.c $(JAVAPATH)es_hpcn_wormhole_Einstein.h $(JAVAPATH)es_hpcn_wormhole_Einstein.cpp
	$(CC)  $(JCFLAGS) $(CFLAGS)   $(JLDFLAGS) -c -o obj/es_hpcn_wormhole_Worm.o $(JAVAPATH)es_hpcn_wormhole_Worm.c
	$(CXX) $(JCFLAGS) $(CXXFLAGS) $(JLDFLAGS) -c -o obj/es_hpcn_wormhole_Einstein.o $(JAVAPATH)es_hpcn_wormhole_Einstein.cpp
	$(CC)  $(JCFLAGS) $(CFLAGS)   $(JLDFLAGS) -shared -o $@ obj/es_hpcn_wormhole_Worm.o obj/es_hpcn_wormhole_Einstein.o

lib/libjavaworm.jar: $(CLASSFILES)
	cd $(JAVAPATH); jar cf libjavaworm.jar $(CLASSFILES) backtype/storm/wh/tests/*class
	mv $(JAVAPATH)libjavaworm.jar lib/libjavaworm.jar

$(JAVAPATH)es_hpcn_wormhole_Worm.h: $(JAVAPATH)es/hpcn/wormhole/Worm.java
	cd $(JAVAPATH) ; rm -f es_hpcn_wormhole_Worm.h ; javah es.hpcn.wormhole.Worm
	
$(JAVAPATH)es_hpcn_wormhole_Einstein.h: $(JAVAPATH)es/hpcn/wormhole/Einstein.java
	cd $(JAVAPATH) ; rm -f es_hpcn_wormhole_Einstein.h ; javah es.hpcn.wormhole.Einstein

.java.class:
	cd $(JAVAPATH); $(JC) $(JFLAGS) $*.java
	
$(JAVAPATH)es/hpcn/wormhole/test/Sentiment.class: $(JAVAPATH)es/hpcn/wormhole/test/Sentiment.java | dependencies/compiled/nlp
	$(JC) $(JFLAGS) -cp "dependencies/compiled/nlp/*" -sourcepath $(JAVAPATH)  $*.java
	
dependencies/compiled/nlp:
	mkdir -p dependencies/compiled
	cd dependencies/compiled ; wget http://nlp.stanford.edu/software/stanford-corenlp-full-2015-12-09.zip
	cd dependencies/compiled ; unzip stanford-corenlp-full-2015-12-09.zip
	cd dependencies/compiled ; rm stanford-corenlp-full-2015-12-09.zip
	cd dependencies/compiled ; rm -rf nlp ; mv stanford* nlp
	
#Common
Dependencies: obj lib bin SSL

dependencies/compiled forceCompileDependencies:
	cd dependencies; $(MAKE)

bin/einstein: src/examples/testEinstein.cpp lib/libworm.so
	$(CXX) $(CXXFLAGS) -o $@ $<  -Llib -lworm $(LDFLAGS)

obj:
	mkdir -p obj
	
certs/prv:
	mkdir -p certs/prv

lib:
	mkdir -p lib

doc/html: $(INCLUDES) $(SRCS) $(JAVAFILES)
	doxygen > /dev/null

bin:
	mkdir -p bin
	ln -s ../certs/ bin/.

libs: lib lib/libworm.so

javaLibs: lib/libjavaworm.so lib/libjavaworm.jar Jexamples

buildTools:
	$(MAKE) -C tools

obj/%.o: src/%.cpp $(INCLUDES) | Dependencies
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/%.o: src/%.c $(INCLUDES) | Dependencies
	$(CC) $(CFLAGS) -c $< -o $@

SSL: certificates dependencies/compiled
certificates: certs/ca.pem certs/worm.pem certs/einstein.pem

export CERTINFOCA=-subj "/C=ES/ST=Madrid/L=Madrid/O=WormHole.ca/CN=www.wormhole.org" 
export CERTINFOWH=-subj "/C=ES/ST=Madrid/L=Madrid/O=WormHole.other/CN=www.wormhole.org" 

certs/prv/%.key.pem: | certs/prv
	openssl ecparam -name brainpoolP512r1 -genkey -noout -out $@ || openssl ecparam -name secp521r1 -genkey -noout -out $@

certs/prv/%.csr: certs/prv/%.key.pem
	openssl req $(CERTINFOCA) -new -key $< -out $@

certs/%.pem: certs/prv/%.csr certs/ca.pem certs/prv/ca.key.pem
	openssl x509 -req -in $< -CA certs/ca.pem -CAkey certs/prv/ca.key.pem -CAcreateserial -out $@ -days 512 -sha512

certs/ca.pem: certs/prv/ca.key.pem
	openssl req $(CERTINFOWH) -x509 -new -nodes -key certs/prv/ca.key.pem -sha512 -days 1024  -extensions v3_ca -out certs/ca.pem


clean:
	rm -rf obj lib bin
	cd $(JAVAPATH); rm -rf $(CLASSFILES)
	./tools/cleanorigs.bash
	cd dependencies; $(MAKE) clean

#Custom Data .o
obj/structures.h.o: $(INCLUDES)
	./tools/parseFile.bash > obj/structures.h
	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 obj/structures.h $@
