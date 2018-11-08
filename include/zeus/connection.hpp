/* Copyright (c) 2015-2018 Rafael Leira, Paula Roquero, Naudit HPCN
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <common.h>
#include <zeus/hole.hpp>

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

namespace zeus
{
class Connection
{
	friend class Zeus;
	friend class ZeusShell;
	friend class ShellCommand;

	std::map<uint16_t, std::shared_ptr<Hole>> connections;
	string listenIpStr;
	uint32_t listenIp;
	uint16_t listenPort;
	unique_ptr<SSocket> listeningSocket;

	// This map represents the holes alredy deployed.
	// The key represents the host, the vector, the programs alredy deployed on it.
	map<string, set<string>> deployedHoles;

	int *holeSockets;  // Sockets for polling
	struct pollfd *fdinfo;
	int numHoleSockets;
	int previousPollIndex;
	int numFilledPolls;
	int startedHoles;

	bool autoDeployHoles = true;

	mutex mtx;
	thread setupThread;

   public:
	Connection(const string listenIp, const uint16_t listenPort);
	Connection(const string listenIp, const uint16_t listenPort, bool autoDeployHoles);
	~Connection();

	// Connects to IP and launches a hole with configuration ws
	void createHole(shared_ptr<Hole> wc);

	void pingHole(const uint16_t id);
	void pingHoles();

	void deleteHole(const uint16_t id);
	void deleteAllHoles();

	void run();

	void mutex_lock();
	void mutex_unlock();

   private:
	// Add socket to hole
	void connectHole(const uint16_t id, unique_ptr<SSocket> socket);
	void pollHoles();
	void listen();
	void threadRun();
	int setupHole();
	void setupHoleThread();
	void deployHole(Hole &wc);

	static void signal_callback_handler(int signum);
	static bool keepRunning;
};
}  // namespace zeus
