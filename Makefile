#VPATH
export JAVAPATH = src/langApis/Java/
export VPATH = $(JAVAPATH)
.SUFFIXES: .java .class
.PRECIOUS: certs/prv/%.csr certs/prv/%.key.pem
.SECONDARY: certs/prv/%.csr certs/prv/%.key.pem

#Common
export TMPDIR=/tmp/

#C/C++
export CC=gcc
export CXX=g++
export FLAGS=-fPIC -I include/ -I dependencies/repos/hptimelib/include -I dependencies/repos/linenoise -Wall -Wextra -g -lpthread -pthread -O3 -Werror
export SSLCFLAGS= -I dependencies/compiled/libressl/usr/local/include
export CFLAGS=$(FLAGS) $(SSLCFLAGS) -std=gnu11
export CXXFLAGS=$(FLAGS) $(SSLCFLAGS) -std=gnu++11
#export SSLLDFLAGS=-lssl -lcrypto
#export SSLLDFLAGS= -Ldependencies/libressl/compiled/usr/local/lib/ -static -lssl -lcrypto #-Wl,-Bdynamic
export SSLLDFLAGS= dependencies/compiled/libressl/usr/local/lib/libtls.a dependencies/compiled/libressl/usr/local/lib/libssl.a dependencies/compiled/libressl/usr/local/lib/libcrypto.a

export LDFLAGS=-fPIC -ldl -lpthread -lstdc++
export LIBWORMLDFLAGS=$(LDFLAGS) dependencies/repos/hptimelib/lib/hptl.a dependencies/repos/linenoise/linenoise.c $(SSLLDFLAGS)

#Einstein
export EINSTEINHEADERS  := $(wildcard src/einstein/*.hpp)
export EINSTEINSRCS     := $(filter-out einstein/main.cpp,$(wildcard src/einstein/*.cpp))
export EINSTEINOBJ      := $(patsubst src/einstein/%,obj/einstein/%,$(EINSTEINSRCS:.cpp=.o))

#Java
export JFLAGS = -g
export JC = javac
export JCFLAGS=-I $(JAVA_HOME)/include/ -I $(JAVA_HOME)/include/linux/
export JLDFLAGS=$(LDFLAGS) -Llib -lworm
export JAVALIBSRC := $(wildcard src/langApis/Java/es/hpcn/wormhole/*.java src/langApis/Java/es/hpcn/wormhole/test/*.java)
export STRMLIBSRC := $(wildcard src/langApis/Java/backtype/storm/*.java src/langApis/Java/backtype/storm/spout/*.java src/langApis/Java/backtype/storm/task/*.java src/langApis/Java/backtype/storm/topology/*.java src/langApis/Java/backtype/storm/topology/base/*.java src/langApis/Java/backtype/storm/tuple/*.java src/langApis/Java/backtype/storm/utils/*.java src/langApis/Java/backtype/storm/generated/*.java src/langApis/Java/backtype/storm/wh/tests/*.java)
export JAVAFILES  := $(JAVALIBSRC) $(STRMLIBSRC)
export CLASSFILES := $(patsubst $(JAVAPATH)%,%,$(JAVAFILES:.java=.class))

export INCLUDES := $(wildcard include/*.h)
export CPPINCLUDES := $(wildcard include/*.h include/*.hpp) $(EINSTEINHEADERS)
export SRCS := $(wildcard src/*.c src/*.cpp src/examples/*.c src/examples/*.cpp)

all: langLibs einstein Examples # doc/html

help Help:
	echo "For a simple compilation try: make simple"

simple: einstein Examples

einstein: bin/einstein

Docs: doc/html

langLibs: javaLibs

Examples: bin/pcapReader.tgz bin/httpDissector.tgz bin/testWorm bin/testLisp bin/testWorm.tgz bin/testLisp.tgz bin/randomEmitter.tgz bin/bandwithMetter.tgz bin/testSendAsync bin/testRecvAsync bin/testSendSSL bin/testRecvSSL bin/testSendAsyncSSL bin/testRecvAsyncSSL
Jexamples: bin/JtestBW.tgz bin/Jtest.tgz

#Tars
bin/%.tgz: bin/% lib/libworm.so | src/examples/runscripts/%.sh
	mkdir -p $(TMPDIR)$(basename $(@F))/lib
	cp $< $(TMPDIR)$(basename $(@F))
	cp -r certs $(TMPDIR)$(basename $(@F))
	cp lib/libworm.so $(TMPDIR)$(basename $(@F))/lib
	cp -L $| $(TMPDIR)$(basename $(@F))/run.sh
	cd $(TMPDIR);	tar -czf $(@F) $(basename $(@F))
	mv $(TMPDIR)/$(@F) $@
	rm -rf $(TMPDIR)$(basename $(@F))

bin/J%.tgz: lib/libworm.so lib/libjavaworm.jar | src/examples/runscripts/J%.sh
	mkdir -p $(TMPDIR)$(basename $(@F))/lib
	cp lib/libjavaworm.* $(TMPDIR)$(basename $(@F))/lib #only for java
	cp -r certs $(TMPDIR)$(basename $(@F))
	cp lib/libworm.so $(TMPDIR)$(basename $(@F))/lib
	cp -L $| $(TMPDIR)$(basename $(@F))/run.sh
	cd $(TMPDIR);	tar -czf $(@F) $(basename $(@F))
	mv $(TMPDIR)/$(@F) $@
	rm -rf $(TMPDIR)$(basename $(@F))
		
bin/nlp.tgz: lib/libworm.so lib/libjavaworm.so lib/libjavaworm.jar src/examples/runscripts/javarun.sh dependencies/compiled/data/data.txt src/langApis/Java/edu/stanford/nlp/sentiment/SentimentPipeline.class
	mkdir -p $(TMPDIR)/nlp/lib
	cp lib/libjavaworm.* $(TMPDIR)/nlp/lib #only for java
	cp lib/libworm.so $(TMPDIR)/nlp/lib
	cp src/examples/runscripts/javarun.sh $(TMPDIR)/nlp/run.sh
	cp -r dependencies/compiled/nlp/* $(TMPDIR)/nlp/lib/.
	cp -r dependencies/compiled/data/data.txt $(TMPDIR)/nlp/
	cd $(TMPDIR); tar -czf nlp.tgz nlp
	mv $(TMPDIR)/nlp.tgz bin/nlp.tgz
	rm -rf $(TMPDIR)/nlp

dependencies/repos/httpDissector/httpDissector_wormhole: lib/libworm.so
	cd dependencies ; $(MAKE) $(MFLAGS) httpDissector

bin/httpDissector.tgz: lib/libworm.so src/examples/runscripts/httpDissector.sh dependencies/repos/httpDissector/httpDissector_wormhole
	mkdir -p $(TMPDIR)httpDissector/lib
	cp dependencies/repos/httpDissector/httpDissector_wormhole $(TMPDIR)httpDissector/httpDissector
	cp lib/libworm.so $(TMPDIR)httpDissector/lib
	cp src/examples/runscripts/httpDissector.sh $(TMPDIR)httpDissector/run.sh
	cd $(TMPDIR);	tar -czf httpDissector.tgz httpDissector
	mv $(TMPDIR)httpDissector.tgz bin/httpDissector.tgz
	rm -rf $(TMPDIR)httpDissector

#Examples
bin/%: src/examples/%.c lib/libworm.so | Dependencies
	$(CC) $(CFLAGS) -o $@ $< -Llib -lworm $(LDFLAGS) $(SSLLDFLAGS)

lib/libworm.so: obj/worm.o obj/common.o obj/netlib.o obj/structures.h.o $(EINSTEINOBJ)
	mkdir -p lib
	$(CC) $(CFLAGS) -shared -o $@ $^  $(LIBWORMLDFLAGS)

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

$(JAVAPATH)es/hpcn/wormhole/test/Sentiment.class: $(JAVAPATH)es/hpcn/wormhole/test/Sentiment.java $(JAVAPATH)edu/stanford/nlp/sentiment/SentimentPipeline.class | dependencies/compiled/nlp
	$(JC) $(JFLAGS) -cp "dependencies/compiled/nlp/*" -sourcepath $(JAVAPATH)  $*.java

$(JAVAPATH)edu/stanford/nlp/sentiment/SentimentPipeline.class: $(JAVAPATH)edu/stanford/nlp/sentiment/SentimentPipeline.java | dependencies/compiled/nlp
	$(JC) $(JFLAGS) -cp "dependencies/compiled/nlp/*" -sourcepath $(JAVAPATH) -sourcepath "dependencies/repos/CoreNLP/src/" $*.java
	cd $(JAVAPATH) ; jar uf ../../../dependencies/compiled/nlp/stanford-corenlp-3.6.0.jar edu/stanford/nlp/sentiment/SentimentPipeline*class


dependencies/compiled/nlp: dependencies/compiled
	mkdir -p dependencies/compiled
	cd dependencies/compiled ; wget http://nlp.stanford.edu/software/stanford-corenlp-full-2015-12-09.zip
	cd dependencies/compiled ; unzip stanford-corenlp-full-2015-12-09.zip
	cd dependencies/compiled ; rm stanford-corenlp-full-2015-12-09.zip
	cd dependencies/compiled ; rm -rf nlp ; mv stanford* nlp
	cd dependencies/repos/CoreNLP ; ../../compiled/gradle/bin/gradle assembleDist || true

#Common
Dependencies: obj bin SSL

dependencies/compiled forceCompileDependencies:
	cd dependencies; $(MAKE) $(MFLAGS)

bin/einstein: src/einstein/main.cpp lib/libworm.so  
	$(CXX) $(CXXFLAGS) -o $@ $< -Llib -lworm $(LDFLAGS)

obj obj/einstein:
	mkdir -p obj/einstein

certs/prv:
	mkdir -p certs/prv

doc/html: $(INCLUDES) $(SRCS) $(JAVAFILES)
	doxygen > /dev/null

bin:
	mkdir -p bin
	ln -s ../certs/ bin/.

javaLibs: lib/libjavaworm.so lib/libjavaworm.jar Jexamples

buildTools:
	$(MAKE) $(MFLAGS) -C tools

obj/%.o: src/%.cpp $(CPPINCLUDES) | Dependencies
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
	cd dependencies; $(MAKE) $(MFLAGS) clean
	git submodule foreach git clean -fdX
	git clean -fdx

#Custom Data .o
obj/structures.h.o: $(INCLUDES)
	./tools/parseFile.bash > obj/structures.h
	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 obj/structures.h $@
