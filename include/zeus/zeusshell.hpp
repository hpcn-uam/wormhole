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
#include <zeus/shellcommand.hpp>
#include <zeus/zeus.hpp>

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
}  // namespace zeus
