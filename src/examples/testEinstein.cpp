#include <einstein.hpp>
#include <common.h>

#include <cassert>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

class EinsteinTester: public Einstein {

 public:
	EinsteinTester();
	~EinsteinTester();
};

EinsteinTester::EinsteinTester() : Einstein("src/examples/test.conf", "0.0.0.0", 5000) {


	assert(ec.listenIp == inet_addr("0.0.0.0"));
	assert(ec.listenPort == 5000);

	assert(ec.connections.at(1)->ws.id == 1);
	assert(ec.connections.at(1)->ws.listenPort == 10000);
	assert(ec.connections.at(1)->ws.core == -1);
	assert(ec.connections.at(1)->ws.IP == inet_addr("127.0.0.1"));
	assert(ec.connections.at(1)->ws.connectionDescriptionLength == 29);
	assert( !memcmp(ec.connections.at(1)->ws.connectionDescription, "(LISP connection description)", 29) );

}
EinsteinTester::~EinsteinTester() {

}



int main (int argc, char **argv) {
	try {
		EinsteinTester einsTester;
		std::cout << "Éxito\n";

	} catch (exception &e) {
		std::cerr << "Excepción: " << e.what() << '\n';
	}
}