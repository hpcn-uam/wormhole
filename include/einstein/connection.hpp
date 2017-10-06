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
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace std;

namespace einstein
{
class Connection
{
	friend class Einstein;
	friend class EinsShell;
	friend class ShellCommand;

	std::map<uint16_t, std::shared_ptr<Worm>> connections;
	string listenIpStr;
	uint32_t listenIp;
	uint16_t listenPort;
	unique_ptr<SSocket> listeningSocket;

	// This map represents the worms alredy deployed.
	// The key represents the host, the vector, the programs alredy deployed on it.
	map<string, set<string>> deployedWorms;

	int *wormSockets;  // Sockets for polling
	struct pollfd *fdinfo;
	int numWormSockets;
	int previousPollIndex;
	int numFilledPolls;
	int startedWorms;

	bool autoDeployWorms = true;

	mutex mtx;
	thread setupThread;

   public:
	Connection(const string listenIp, const uint16_t listenPort);
	Connection(const string listenIp, const uint16_t listenPort, bool autoDeployWorms);
	~Connection();

	// Connects to IP and launches a worm with configuration ws
	void createWorm(shared_ptr<Worm> wc);

	void pingWorm(const uint16_t id);
	void pingWorms();

	void deleteWorm(const uint16_t id);
	void deleteAllWorms();

	void run();

	void mutex_lock();
	void mutex_unlock();

   private:
	// Add socket to worm
	void connectWorm(const uint16_t id, unique_ptr<SSocket> socket);
	void pollWorms();
	void listen();
	void threadRun();
	int setupWorm();
	void setupWormThread();
	void deployWorm(Worm &wc);

	static void signal_callback_handler(int signum);
	static bool keepRunning;
};
}

#endif
