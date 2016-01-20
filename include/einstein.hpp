#ifndef __EINSTEIN_H__
#define __EINSTEIN_H__

#include "common.h"
#include <iostream>

using namespace std;

struct Eins2WormConn
{
	WormSetup ws;
	int socket;
};

class EinsConn
{
	map <uint16_t, Eins2WormConn> connections;
	int listeningSocket;

 public:
	EinsConn(uint32_t listenIP, uint16_t listenPort);
	~EinsConn();

	// Connects to IP and launches a worm with configuration ws
	void createWorm(const WormSetup ws, const uint32_t IP);

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
	Einstein(const string configFileName);
	~Einstein();

	void openHoles(); // Starts everything

 private:
	void readConfig(const string configFileName);

};

#endif
