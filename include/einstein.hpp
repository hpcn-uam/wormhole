#ifndef __EINSTEIN_H__
#define __EINSTEIN_H__

#include <common.h>
#include <iostream>
#include <map>
#include <memory>
#include <poll.h>

using namespace std;

struct Eins2WormConn {
	WormSetup ws;
	int socket;
	string host;
	string programName;

	Eins2WormConn(uint16_t id, uint16_t listenPort, int16_t core, string ip, string connectionDescription, string host, string programName);
	~Eins2WormConn();
};

class EinsConn
{
	friend class EinsteinTester;
	friend class Einstein;
	map <uint16_t, unique_ptr<Eins2WormConn>> connections;
	string listenIpStr;
	uint32_t listenIp;
	uint16_t listenPort;
	int listeningSocket;

	int *wormSockets; // Sockets for polling
	struct pollfd *fdinfo;
	int numWormSockets;
	int previousPollIndex;
	int numFilledPolls;
	
	bool autoDeployWorms = true;

 public:
	EinsConn(const string listenIp, const uint16_t listenPort);
	EinsConn(const string listenIp, const uint16_t listenPort, bool autoDeployWorms);
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
	void pollWorms();
	void listen();
	void threadRun();
	int setupWorm();
	void deployWorm(Eins2WormConn &wc);
};

class Einstein
{
	friend class EinsteinTester;
	friend class EinsConn;
	EinsConn ec;

 public:
	Einstein(const string configFileName, string listenIp, uint16_t listenPort);
	Einstein(const string configFileName, string listenIp, uint16_t listenPort, bool autoDeployWorms);
	~Einstein();

	void openHoles(); // Starts everything

 private:
	void readConfig(const string configFileName);
};

#endif
