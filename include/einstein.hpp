#ifndef __EINSTEIN_H__
#define __EINSTEIN_H__

#include <common.h>

#include <poll.h>
#include <signal.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <stdexcept>

using namespace std;

struct Eins2WormConn {
	WormSetup ws;
	int socket;
	string host;
	string programName;
	bool halting;
	bool deployed;

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

	vector<string> runParams;

	// This map represents the worms alredy deployed.
	// The key represents the host, the vector, the programs alredy deployed on it.
	map <string, set<string>> deployedWorms;

	int *wormSockets; // Sockets for polling
	struct pollfd *fdinfo;
	int numWormSockets;
	int previousPollIndex;
	int numFilledPolls;

	bool autoDeployWorms = true;

 public:
	EinsConn(const string listenIp, const uint16_t listenPort);
	EinsConn(const string listenIp, const uint16_t listenPort, bool autoDeployWorms);
	EinsConn(const string listenIp, const uint16_t listenPort, bool autoDeployWorms, vector<string> runParams);
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

	static void signal_callback_handler(int signum);
	static bool keepRunning;
};

class Einstein
{
	friend class EinsteinTester;
	friend class EinsConn;
	EinsConn ec;

 public:
	Einstein(const string configFileName, string listenIp, uint16_t listenPort);
	Einstein(const string configFileName, string listenIp, uint16_t listenPort, bool autoDeployWorms);
	Einstein(const string configFileName, string listenIp, uint16_t listenPort, bool autoDeployWorms, vector<string> runParams);
	~Einstein();

	void openHoles(); // Starts everything

 private:
	void readConfig(const string configFileName);
};

#endif
