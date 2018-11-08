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
#include <zeus/zeus.hpp>

#include <poll.h>
#include <signal.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <locale>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

using namespace std;

namespace zeus
{
class ShellCommand
{
	friend class ZeusShell;

   public:
	// variables
	static shared_ptr<Zeus> zeus;
	string cmd;
	function<int(string)> exec;

	string shortHelp;
	string longHelp;
	string hints;

	// set<ShellCommand> subCommands;

	// methods
	ShellCommand(string cmd, function<int(string)> exec, string shortHelp, string longHelp);

	ShellCommand(string cmd, function<int(string)> exec, string shortHelp, string longHelp, string hits);

	~ShellCommand();

	bool operator<(const ShellCommand &rhs) const;

	int execute(string cmd);

	// static methods
	static set<ShellCommand> getCommandList();

	// Commands for methods
	static int cmdHelp(string cmd);
	static int cmdHalt(string cmd);
	static int cmdQuit(string cmd);
	static int cmdList(string cmd);
	static int cmdPing(string cmd);
	static int cmdKill(string cmd);
	static int cmdChRoute(string cmd);
	static int cmdVersion(string cmd);
	static int cmdStatistics(string cmd);

   private:
	ShellCommand(string cmd);
	static int forHole(string cmd, function<int(shared_ptr<Hole>, string)> fn);
	static string normalize(string str);
	static string normalize(string str, int length);
};
}  // namespace zeus
