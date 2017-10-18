#pragma once

#include <common.h>
#include <netlib.hpp>

#include <poll.h>
#include <signal.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

using namespace std;

// fix to einsshell
#define endl "\r" << endl

namespace einstein
{
struct Worm {
   public:
	WormSetup ws;
	unique_ptr<SSocket> socket;
	string host;
	string programName;
	bool halting;
	bool deployed;

	vector<string> runParams;

	Worm(uint16_t id, uint16_t listenPort, int16_t core, string connectionDescription, string host, string programName);
	Worm(uint16_t id,
	     uint16_t listenPort,
	     int16_t core,
	     string connectionDescription,
	     string host,
	     string programName,
	     vector<string> runParams);
	~Worm();

	void setIP(string iphostname);
	static string expandCDescription(string cd);

	int64_t ping();                     // returns the ms passed from the ping. If negative, timeout reached
	uint64_t chroute(string newRoute);  // returns 0 if changed, and 1 if not
};

ostream &operator<<(ostream &os, Worm const &obj);
}
