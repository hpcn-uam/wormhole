#include <einstein.hpp>
#include <cstring>

#include <arpa/inet.h>

Eins2WormConn::Eins2WormConn(uint16_t id, uint16_t listenPort, uint16_t core, string ip, string connectionDescription) {
	this->ws.id = id;
	this->ws.listenPort = listenPort;
	this->ws.core = core;
	this->ws.IP = inet_addr(ip.c_str());
	this->ws.connectionDescriptionLength = connectionDescription.size();
	this->ws.connectionDescription = static_cast<uint8_t *>(malloc(connectionDescription.size()));
	memcpy(this->ws.connectionDescription, connectionDescription.c_str(), connectionDescription.size());
}

Eins2WormConn::~Eins2WormConn() {
	free(this->ws.connectionDescription);
}

Einstein::Einstein(const string configFileName, const string listenIp, const uint16_t listenPort)
		: ec(listenIp, listenPort) {
		
	this->readConfig(configFileName);
	ec.run();
}

Einstein::~Einstein() {
}

void Einstein::readConfig(const string configFileName) {
	// TODO: Leer realmente el fichero
	uint16_t id = 1;
	uint16_t listenPort = 10000;
	uint16_t core = 0;
	string ip = "127.0.0.1";
	string connectionDescription = "(LISP connection description)";

	unique_ptr<Eins2WormConn> wc(new Eins2WormConn(id, listenPort, core, ip, connectionDescription));
	
	this->ec.createWorm(std::move(wc), ip);
}

EinsConn::EinsConn(string listenIp, uint16_t listenPort) {
	
}

EinsConn::~EinsConn() {
	this->deleteAllWorms();
}

void EinsConn::createWorm(unique_ptr<Eins2WormConn> wc, const string ip) {
	// TODO: Conectarse al remoto y crear worm
	
	this->connections.insert(make_pair(wc->ws.id, std::move(wc)));
}