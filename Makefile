export INCLUDES := $(wildcard include/*.h include/*hpp)
export SRCS := $(wildcard src/*.c src/*.cpp src/einstein/*c src/einstein/*cpp src/examples/*.c src/examples/*.cpp)

all: $(INCLUDES) $(SRCS) | build 
	cd build && cmake ..
	cd build && ${MAKE} --no-print-directory

clean: build
	cd build && ${MAKE} clean --no-print-directory

build:
	mkdir -p build

Docs: doc/html

doc/html: $(INCLUDES) $(SRCS) #$(JAVAFILES)
	doxygen > /dev/null