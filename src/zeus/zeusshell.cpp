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

#include <zeus/zeusshell.hpp>

using namespace zeus;

set<ShellCommand> ZeusShell::commands;

extern "C" {
void completeln(const char *buf, linenoiseCompletions *lc)
{
	// search for similar:
	string temp = string(buf);

	transform(temp.begin(), temp.end(), temp.begin(), [](char x) { return toupper(x, locale()); });

	for (auto element : ZeusShell::commands) {
		string transfelement = element.cmd.substr(0, temp.length());

		transform(
		    transfelement.begin(), transfelement.end(), transfelement.begin(), [](char x) { return toupper(x, locale()); });

		if (transfelement == temp) {
			linenoiseAddCompletion(lc, (element.cmd + " ").c_str());
		}
	}
}

char *hintsln(const char *buf, int *color, int *bold)
{
	// search for similar:
	string temp = string(buf);

	transform(temp.begin(), temp.end(), temp.begin(), [](char x) { return toupper(x, locale()); });

	for (auto element : ZeusShell::commands) {
		string transfelement = element.cmd;

		transform(
		    transfelement.begin(), transfelement.end(), transfelement.begin(), [](char x) { return toupper(x, locale()); });

		string shortedElement = transfelement.substr(0, temp.length());

		if (transfelement == temp) {
			*bold  = 1;
			*color = 35;
			return strdup((" " + element.hints).c_str());  // show hints if full word

		} else if (transfelement + " " == temp) {
			*bold  = 1;
			*color = 35;
			return strdup((element.hints).c_str());  // show hints if complete word + space

		} else if (shortedElement == temp) {  // TODO: consider remove, seems to be confusing
			*bold  = 0;
			*color = 34;
			return strdup(element.cmd.substr(temp.length(), element.cmd.length()).c_str());  // complete to the closest word
		}
	}

	return NULL;
}

void hintsFree(void *elem)
{
	if (elem) {
		free(elem);
	}
}
}

ZeusShell::ZeusShell(shared_ptr<Zeus> zeus)
{
	this->zeus          = zeus;
	this->prompt        = "Zeus> ";
	this->historyPath   = string(getenv("HOME")) + "/.zeus.hist";
	this->historyLength = 500;
	this->continueShell = true;

	this->commands = ShellCommand::getCommandList();

	linenoiseHistorySetMaxLen(this->historyLength);
	linenoiseHistoryLoad(this->historyPath.c_str());
	linenoiseClearScreen();
	linenoiseSetMultiLine(1);

	linenoiseSetCompletionCallback(completeln);
	linenoiseSetHintsCallback(hintsln);
	linenoiseSetFreeHintsCallback(hintsFree);

	ShellCommand::zeus = zeus;
}

ZeusShell::~ZeusShell()
{
	linenoiseHistorySetMaxLen(1);
	linenoiseHistorySetMaxLen(0);
}

int ZeusShell::startShell()
{
	int ret = 0;

	while (this->continueShell && this->zeus->ec.keepRunning) {
		char *tmp = linenoise(this->prompt.c_str());

		if (tmp != NULL && tmp[0] != 0) {
			linenoiseHistoryAdd(tmp);

			if (linenoiseHistorySave(this->historyPath.c_str())) {
				perror("WTF");
				cerr << "Cant save history to: " << this->historyPath.c_str() << endl;
			}

			int result = this->executeCmd(string(tmp));

			if (result == 1) {
				ret                 = 0;
				this->continueShell = false;

			} else if (result) {
				ret                 = 1;
				this->continueShell = false;
			}
		}

		if (tmp) {
			linenoiseFree(tmp);
		}
	}

	return ret;
}

void ZeusShell::waitForZeus()
{
	while (zeus->ec.numFilledPolls < zeus->ec.numHoleSockets && zeus->ec.keepRunning) {
		hptl_waitns(10 * 1000 * 1000UL);
	}
}

int ZeusShell::executeCmd(string cmd)
{
	auto pos         = cmd.find(' ');
	string searchcmd = cmd;

	if (pos != string::npos) {
		searchcmd = cmd.substr(0, pos);
	}

	auto found = this->commands.find(ShellCommand(searchcmd));

	if (found != this->commands.end()) {
		this->zeus->mutex_lock();
		int ret = found->exec(cmd);
		this->zeus->mutex_unlock();

		return ret;

	} else {
		cout << "Command \"" << cmd << "\" not available" << endl;
	}

	return 0;
}
