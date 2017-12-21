#pragma once

#include <common.h>
#include <einstein/einstein.hpp>
#include <einstein/shellcommand.hpp>

#include <linenoise.h>

#include <poll.h>
#include <signal.h>

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

namespace einstein
{
class EinsShell
{
   public:
	// variables
	shared_ptr<Einstein> eins;
	static set<ShellCommand> commands;

	string prompt;
	string historyPath;
	int historyLength;
	bool continueShell;

	// methods
	EinsShell(shared_ptr<Einstein> eins);
	~EinsShell();

	/*
	 * Starts the Einstein's Shell
	 * @return 0 if normal exit. Other or exception, if some error.
	 */
	int startShell();

	/*
	 * Waits until einstein successfully starts
	 */
	void waitForEinstein();

   private:
	int executeCmd(string cmd);
};
}
