#include <einstein/worm.hpp>

using namespace einstein;

Worm::Worm(uint16_t id, uint16_t listenPort, int16_t core, string ip, string connectionDescription, string host, string programName)
{
	connectionDescription = Worm::expandCDescription(connectionDescription);
	memset(&this->ws, 0, sizeof(this->ws));

	this->ws.id = id;
	this->ws.listenPort = listenPort;
	this->ws.IP = inet_addr(ip.c_str());
	this->ws.connectionDescriptionLength = connectionDescription.size();
	this->ws.connectionDescription = static_cast<uint8_t *>(malloc(connectionDescription.size()));
	this->ws.core = core;
	this->ws.isSSLNode = 0; //false
	memcpy(this->ws.connectionDescription, connectionDescription.c_str(), connectionDescription.size());
	this->host = host;
	this->programName = programName;
	this->halting = false;
	this->deployed = false;
}

Worm::~Worm()
{
	free(this->ws.connectionDescription);

	if (this->deployed) {
		ctrlMsgType msg = HALT;
		cerr << "Enviando HALT al Worm id = " << this->ws.id << endl;

		if (tcp_message_send(this->socket, &msg, sizeof(msg)) != 0) {
			throw std::runtime_error("Error sending HALT");
		}

	} else {
		cerr << "Worm with id = " << this->ws.id << " has not alredy been deployed, do not sending HALT..." << endl;
	}
}

ostream &einstein::operator<<(ostream &os, Worm const &obj)
{
	return os
		   <<  "ID: " << obj.ws.id
		   << " ADDR: " <<  obj.host << ":" << obj.ws.listenPort
		   << (obj.ws.isSSLNode ? " [SSL]" : "")
		   << " | " << obj.programName << " " << endl
		   << "\t Route: " << obj.ws.connectionDescription
		   ;
}

string Worm::expandCDescription(string cd)
{
	string ret = cd;

	//clean the string
	char removableChars[] = "\t\r\n\"'";

	for (unsigned int i = 0; i < strlen(removableChars); i++) {
		ret.erase(remove(ret.begin(), ret.end(), removableChars[i]), ret.end());
	}

	return ret;
}

uint64_t Worm::ping()
{
	if (this->socket == 0 || this->socket == -1) {
		return 0;
	}

	hptl_t begin = hptl_get();

	ctrlMsgType msg = PING;
	tcp_message_send(this->socket, &msg, sizeof(msg));
	tcp_message_recv(this->socket, &msg, sizeof(msg), 0);

	hptl_t end = hptl_get();

	return hptl_ntimestamp(end - begin) / 1000;
}


uint64_t Worm::chroute(string newRoute)
{
	if (this->socket == 0 || this->socket == -1) {
		return 1;
	}

	newRoute = Worm::expandCDescription(newRoute);

	ctrlMsgType msg = CHANGEROUTE;
	uint32_t length = newRoute.length() + 1;
	tcp_message_send(this->socket, &msg, sizeof(msg));
	tcp_message_send(this->socket, &length, sizeof(length));
	tcp_message_send(this->socket, newRoute.c_str(), length);

	this->ws.connectionDescriptionLength = length;
	this->ws.connectionDescription = (uint8_t *)realloc(this->ws.connectionDescription, length);
	memcpy(this->ws.connectionDescription, newRoute.c_str(), length);

	return 0;
}
