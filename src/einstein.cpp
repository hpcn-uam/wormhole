#include <einstein.hpp>

#include <cstring>
#include <cstdio>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>

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
	// Start socket to receive connections from worms
	this->listenIp = inet_addr(listenIp.c_str());
	this->listenPort = listenPort;
	this->listeningSocket = tcp_listen_on_port(listenPort);

	if (this->listeningSocket == -1) {
		throw std::runtime_error("Error listening to socket");
	}
}

EinsConn::~EinsConn() {
	//this->deleteAllWorms();
	close(this->listeningSocket);
	close(this->currentWormSocket);
}

void EinsConn::createWorm(unique_ptr<Eins2WormConn> wc, const string ip) {
	// TODO: Conectarse al remoto y crear worm

	this->connections.insert(make_pair(wc->ws.id, std::move(wc)));
}

void EinsConn::run() {

	// Wait for connections from worms
	for(size_t i = 0; i < this->connections.size(); i++) {
		int currentWormSocket = tcp_accept(this->listeningSocket);
		if (currentWormSocket == -1) {
			throw std::runtime_error("Error accepting connection");
		}

		// Get hello message
		size_t hellomsgSize = sizeof(enum ctrlMsgType) + sizeof(uint16_t);
		uint8_t hellomsg[hellomsgSize];

		if (tcp_message_recv(currentWormSocket, hellomsg, hellomsgSize) != 0) {
			throw std::runtime_error("Error receiving message");
		}
		if (*((enum ctrlMsgType *) &hellomsg) != HELLOEINSTEIN) {
			continue;
		}

		uint16_t wormId = ntohs(*((uint16_t *) (hellomsg + sizeof(enum ctrlMsgType))));
		this->connections.at(wormId)->socket = currentWormSocket;

		// Send configuration message
		const void *wormSetup = static_cast<const void *>(&(this->connections.at(wormId)->ws));
		if (tcp_message_send(currentWormSocket, wormSetup, sizeof(WormSetup)) != 0) {
			throw std::runtime_error("Error sending message");
		}
		const void *connDescription = static_cast<const void *>(this->connections.at(wormId)->ws.connectionDescription);
		if (tcp_message_send(currentWormSocket, connDescription, this->connections.at(wormId)->ws.connectionDescriptionLength) != 0) {
			throw std::runtime_error("Error sending message");
		}
	}
}