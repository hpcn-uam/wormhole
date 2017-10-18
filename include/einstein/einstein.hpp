#pragma once

#include <common.h>
#include <einstein/connection.hpp>

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

namespace einstein
{
class Einstein
{
	friend class Connection;
	friend class EinsShell;
	friend class ShellCommand;
	Connection ec;
	thread thr;

   public:
	Einstein(const string configFileName, string listenIp, uint16_t listenPort);
	Einstein(const string configFileName, string listenIp, uint16_t listenPort, bool autoDeployWorms);
	~Einstein();

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
