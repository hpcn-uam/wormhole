#include <einstein/einsshell.hpp>

#include <linenoise.h>

using namespace einstein;

EinsShell::EinsShell(shared_ptr<Einstein> eins)
{
	this->eins  		= eins;
	this->prompt		= "Einstein> ";
	this->historyPath	= "~/.einstein.history";
	this->historyLength	= 500;
	this->continueShell	= true;

	this->commands = ShellCommand::getCommandList();

	linenoiseHistorySetMaxLen(this->historyLength);
	linenoiseHistoryLoad(this->historyPath.c_str());
	linenoiseClearScreen();
	linenoiseSetMultiLine(1);
}

EinsShell::~EinsShell()
{

}

int EinsShell::startShell()
{
	int ret = 0;

	while (this->continueShell) {
		char *tmp = linenoise(this->prompt.c_str());

		if (tmp != NULL && tmp[0] != 0) {
			linenoiseHistoryAdd(tmp);
			linenoiseHistorySave(this->historyPath.c_str());

			if (this->executeCmd(string(tmp))) {
				ret = 1;
				this->continueShell = false;
			}
		}

		if (tmp) {
			linenoiseFree(tmp);
		}
	}

	return ret;
}

int EinsShell::executeCmd(string cmd)
{
	auto pos = cmd.find(' ');
	string searchcmd = cmd;

	if (pos != string::npos) {
		searchcmd = cmd.substr(0, pos);
	}

	auto found = this->commands.find(ShellCommand(searchcmd));

	if (found != this->commands.end()) {
		return found->exec(cmd);

	} else {
		cout << "Command \"" << cmd << "\" not available" << endl;
	}

	return 0;
}