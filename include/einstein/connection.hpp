#ifndef __EINSTEIN_CONNECTION_H__
#define __EINSTEIN_CONNECTION_H__

#include <common.h>
#include <einstein/worm.hpp>

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

namespace einstein
{
class Connection
{
	friend class Einstein;
	friend class EinsShell;
	friend class ShellCommand;

	map <uint16_t, shared_ptr<Worm>> connections;
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
	int startedWorms;

	bool autoDeployWorms = true;

 public:
	Connection(const string listenIp, const uint16_t listenPort);
	Connection(const string listenIp, const uint16_t listenPort, bool autoDeployWorms);
	Connection(const string listenIp, const uint16_t listenPort, bool autoDeployWorms, vector<string> runParams);
	~Connection();

	// Connects to IP and launches a worm with configuration ws
	void createWorm(shared_ptr<Worm> wc, const string ip);

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
	void deployWorm(Worm &wc);

	static void signal_callback_handler(int signum);
	static bool keepRunning;
};
}

#endif
