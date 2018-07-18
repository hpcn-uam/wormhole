#pragma once

#include <common.h>
#include <zeus/connection.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace std;

namespace zeus
{
class Zeus
{
	friend class Connection;
	friend class ZeusShell;
	friend class ShellCommand;
	Connection ec;
	thread thr;

   public:
	Zeus(const string configFileName, string listenIp, uint16_t listenPort);
	Zeus(const string configFileName, string listenIp, uint16_t listenPort, bool autoDeployHoles);
	~Zeus();

	/*
	 *Starts everything SYNCRONIOUSLY
	 */
	void openHoles();

	/*
	 *Starts everything ASYNCRONIOUSLY
	 */
	void openThreadedHoles();

	void mutex_lock();
	void mutex_unlock();

   private:
	void readConfig(const string configFileName);
};
}
