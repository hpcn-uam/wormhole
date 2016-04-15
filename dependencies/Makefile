export CURRDIR=$(shell pwd)

JAVAPATH=""
VPATH=""
INCLUDES=""
SRCS=""
FLAGS=""
CFLAGS=""
CXXFLAGS=""
LDFLAGS=""

all: LibreSSL

#Common
compiled:
	mkdir -p compiled

initRepositories: checkRepositories

checkRepositories: repos/hptimelib/README.md

repos/hptimelib/README.md:
	cd ..; git submodule update --init --recursive

#LibreSSL
LibreSSL: compiled/libressl/usr

export LIBSSLCFLAGS='-O3 -fPIC'
compiled/libressl: repos/libressl/Makefile | compiled
	mkdir -p compiled compiled/libressl

compiled/libressl/usr: compiled/libressl
	cd repos/libressl; make install DESTDIR=$(CURRDIR)/compiled/libressl

repos/libressl/openbsd: initRepositories | repos/libressl-dep/
	rm -f repos/libressl/openbsd
	ln -s ../libressl-dep repos/libressl/openbsd

repos/libressl/autogen.sh: | repos/libressl/openbsd initRepositories

repos/libressl/configure: repos/libressl/autogen.sh
	cd repos/libressl; ./autogen.sh

repos/libressl/Makefile: repos/libressl/configure
	cd repos/libressl; ./configure CFLAGS=$(LIBSSLCFLAGS)

clean:
	cd .. ; git submodule update --init --recursive
	cd .. ; git submodule update --recursive #its really needed?
	git submodule foreach git reset --hard
	git submodule foreach git clean -f -d
	rm -rf compiled
	cd repos/libressl ; make clean #clean libressl
