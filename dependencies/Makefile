export CURRDIR=$(shell pwd)

JAVAPATH=
VPATH=
INCLUDES=
SRCS=
FLAGS=
CFLAGS=
CXXFLAGS=
LDFLAGS=

unexport JAVAPATH
unexport VPATH
unexport INCLUDES
unexport SRCS
unexport FLAGS
unexport CFLAGS
unexport CXXFLAGS
unexport LDFLAGS

all: LibreSSL GRADLE HPTLib

#Common
compiled:
	mkdir -p compiled

initRepositories: checkRepositories

checkRepositories: repos/hptimelib/README.md

repos/hptimelib/README.md:
	cd ..; git submodule update --init --recursive

#LibreSSL
LibreSSL: compiled/libressl/usr

GRADLE: compiled/gradle

HPTLib: repos/hptimelib/lib/hptl.a

repos/hptimelib/lib/hptl.a: | initRepositories
	cd repos/hptimelib; make

compiled/gradle:
	cd compiled ; rm -rf gradle
	cd compiled ; wget https://services.gradle.org/distributions/gradle-2.14-bin.zip
	cd compiled ; unzip gradle*.zip
	cd compiled ; rm gradle*.zip
	cd compiled ; mv gradle* gradle

httpDissector: repos/httpDissector/httpDissector_wormhole ../lib/libworm.so

repos/httpDissector/httpDissector_wormhole: repos/httpDissector/httpDissector.c ../lib/libworm.so
	cd repos/httpDissector ; make httpDissector_wormhole

repos/httpDissector/httpDissector.c: initRepositories

export LIBSSLCFLAGS='-O3 -fPIC'
compiled/libressl: repos/libressl/Makefile | compiled
	mkdir -p compiled/libressl

compiled/libressl/usr: compiled/libressl
	cd repos/libressl; $(MAKE) $(MFLAGS) install DESTDIR=$(CURRDIR)/compiled/libressl

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
	cd repos/libressl ; $(MAKE) $(MFLAGS) clean #clean libressl
	cd repos/libressl ; rm -rf openbsd configure Makefile
