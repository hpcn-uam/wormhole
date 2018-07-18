export INCLUDES := $(wildcard include/*.h include/*hpp)
export SRCS := $(wildcard src/*.c src/*.cpp src/zeus/*c src/zeus/*cpp src/examples/*.c src/examples/*.cpp)

CMAKE ?= cmake

all: $(INCLUDES) $(SRCS) | build 
	git submodule update --init --recursive
	cd build && $(CMAKE) ..
	cd build && ${MAKE} --no-print-directory

clean: build
	cd build && ${MAKE} clean --no-print-directory

build:
	mkdir -p build

Docs: doc/html

doc/html: $(INCLUDES) $(SRCS) #$(JAVAFILES)
	doxygen > /dev/null