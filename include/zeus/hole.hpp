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

// fix to zeusshell
#define endl "\r" << endl

namespace zeus
{
struct Hole {
   public:
	HoleSetup ws;
	unique_ptr<SSocket> socket;
	string host;
	string programName;
	bool halting;
	bool autoDeploy;
	bool deployed;

	vector<string> runParams;

	Hole(uint16_t id, uint16_t listenPort, int16_t core, string connectionDescription, string host, string programName);
	Hole(uint16_t id,
	     uint16_t listenPort,
	     int16_t core,
	     string connectionDescription,
	     string host,
	     string programName,
	     vector<string> runParams);
	~Hole() noexcept(false);

	void setIP(string iphostname);
	bool setTimeoutResponse(time_t seconds);  // returns true if applied, false if failed
	static string expandCDescription(string cd);

	int64_t ping();                     // returns the ms passed from the ping. If negative, timeout reached
	int64_t kill();                     // returns 0 if killed, and 1 if not
	uint64_t chroute(string newRoute);  // returns 0 if changed, and 1 if not
#ifdef WH_STATISTICS
	vector<ConnectionStatistics> getStatistics(
	    bool inout);  // returns the hole statistics. If innout is true, input statistics, else, output statistics
#endif
};

ostream &operator<<(ostream &os, Hole const &obj);
}  // namespace zeus
