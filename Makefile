all: $(wildcard src/*c src/*cpp src/einstein/*c src/einstein/*cpp src/examples/*c src/examples/*cpp include/*h include/*hpp) | build 
	cd build && cmake ..
	cd build && ${MAKE} --no-print-directory

build:
	mkdir -p build
