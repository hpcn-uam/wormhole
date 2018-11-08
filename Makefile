# Copyright (c) 2015-2018 Rafael Leira
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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