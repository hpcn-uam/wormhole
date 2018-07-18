#pragma once

#include <common.h>
#include <zeus/zeus.hpp>
#include <zeus/shellcommand.hpp>

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

namespace zeus
{
class ZeusShell
{
   public:
	// variables
	shared_ptr<Zeus> zeus;
	static set<ShellCommand> commands;

	string prompt;
	string historyPath;
	int historyLength;
	bool continueShell;

	// methods
	ZeusShell(shared_ptr<Zeus> zeus);
	~ZeusShell();

	/*
	 * Starts the Zeus's Shell
	 * @return 0 if normal exit. Other or exception, if some error.
	 */
	int startShell();

	/*
	 * Waits until zeus successfully starts
	 */
	void waitForZeus();

   private:
	int executeCmd(string cmd);
};
}
