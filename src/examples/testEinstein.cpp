#include <einstein.hpp>
#include <common.h>

#include <cassert>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

class EinsteinTester: public Einstein
{

 public:
	EinsteinTester();
	~EinsteinTester();
};

EinsteinTester::EinsteinTester() : Einstein("../src/examples/test.conf", "150.244.58.77", 5000)
{


	assert(ec.listenIp == inet_addr("150.244.58.77"));
	assert(ec.listenPort == 5000);

	assert(ec.connections.at(1)->ws.id == 1);
	assert(ec.connections.at(1)->ws.listenPort == 10000);
	assert(ec.connections.at(1)->ws.core == -1);
	assert(ec.connections.at(1)->ws.IP == inet_addr("150.244.58.77"));
	assert(ec.connections.at(1)->ws.connectionDescriptionLength == 29);
	assert(!memcmp(ec.connections.at(1)->ws.connectionDescription, "1", 29));

}
EinsteinTester::~EinsteinTester()
{

}



int main(int argc, char **argv)
{
	try {
		EinsteinTester einsTester;
		std::cout << "Éxito\n";

	} catch (exception &e) {
		std::cerr << "Excepción: " << e.what() << '\n';
	}
}
