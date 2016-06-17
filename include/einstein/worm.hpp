#ifndef __EINSTEIN_WORM_H__
#define __EINSTEIN_WORM_H__

#include <common.h>

#include <poll.h>
#include <signal.h>

#include <algorithm>
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

//fix to einsshell
#define endl "\r"<<endl

namespace einstein
{
struct Worm {
 public:
	WormSetup ws;
	int socket;
	string host;
	string programName;
	bool halting;
	bool deployed;

	vector<string> runParams;

	Worm(uint16_t id, uint16_t listenPort, int16_t core, string ip, string connectionDescription, string host, string programName);
	Worm(uint16_t id, uint16_t listenPort, int16_t core, string ip, string connectionDescription, string host, string programName, vector<string> runParams);
	~Worm();

	static string expandCDescription(string cd);

	uint64_t ping(); //returns the ms passed from the ping
	uint64_t chroute(string newRoute); //returns 0 if changed, and 1 if not
};

ostream &operator<<(ostream &os, Worm const &obj);
}

#endif
