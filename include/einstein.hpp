#ifndef __EINSTEIN_H__
#define __EINSTEIN_H__

#include <common.h>
#include <iostream>
#include <map>
#include <memory>

using namespace std;

struct Eins2WormConn
{
	WormSetup ws;
	int socket;
	
	Eins2WormConn(uint16_t id, uint16_t listenPort, uint16_t core, string ip, string connectionDescription);
	~Eins2WormConn();
};

class EinsConn
{
	map <uint16_t, unique_ptr<Eins2WormConn>> connections;
	uint32_t listenIp;
	uint16_t listenPort;
	int listeningSocket;

 public:
	EinsConn(const string listenIp, const uint16_t listenPort);
	~EinsConn();

	// Connects to IP and launches a worm with configuration ws
	void createWorm(unique_ptr<Eins2WormConn> wc, const string ip);

	void pingWorm(const uint16_t id);
	void pingWorms();

	void deleteWorm(const uint16_t id);
	void deleteAllWorms();

	void run();

 private:
	// Add socket to worm
	void connectWorm(const uint16_t id, const int socket);
	void threadRun();
};

class Einstein
{
	EinsConn ec;

 public:
	Einstein(const string configFileName, string listenIp, uint16_t listenPort);
	~Einstein();

	void openHoles(); // Starts everything

 private:
	void readConfig(const string configFileName);

};

#endif
