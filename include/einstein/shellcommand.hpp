#ifndef __EINSTEIN_SHELLCOMMAND_H__
#define __EINSTEIN_SHELLCOMMAND_H__

#include <common.h>
#include <einstein/einstein.hpp>

#include <poll.h>
#include <signal.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <stdexcept>

using namespace std;

namespace einstein
{
class ShellCommand
{
	friend class EinsShell;

	string cmd;
	function<int(string)> exec;

	string shortHelp;
	string longHelp;

 public:
	ShellCommand(
		string cmd,
		function<int(string)> exec,
		string shortHelp,
		string longHelp);
	~ShellCommand();

	bool operator< (const ShellCommand &rhs) const;

	int execute(string cmd);

	//static methods
	static set<ShellCommand> getCommandList();

	//Commands for methods
	static int cmdHelp(string cmd);
	static int cmdHalt(string cmd);
 private:
	ShellCommand(string cmd);
};
}

#endif
