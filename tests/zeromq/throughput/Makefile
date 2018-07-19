all: client server

%: %.cpp
	g++ ${FLAGS} $< -o $@ -lzmq -I ../../../dependencies/repos/hptimelib/include/ -I ../../../include/ ../../../build/lib/libhptl.a -lpthread

clean:
	rm -f client server