#include <einstein/einsshell.hpp>

using namespace einstein;

set<ShellCommand> EinsShell::commands;

extern "C"
{
	void completeln(const char *buf, linenoiseCompletions *lc)
	{
		//search for similar:
		string temp = string(buf);

		transform(temp.begin(), temp.end(), temp.begin(), [](char x) {
			return toupper(x, locale());
		});

	for (auto element : EinsShell::commands) {
			string transfelement = element.cmd.substr(0, temp.length());

			transform(transfelement.begin(), transfelement.end(), transfelement.begin(), [](char x) {
				return toupper(x, locale());
			});

			if (transfelement == temp) {
				linenoiseAddCompletion(lc, (element.cmd + " ").c_str());
			}
		}
	}

	char *hintsln(const char *buf, int *color, int *bold)
	{
		//search for similar:
		string temp = string(buf);

		transform(temp.begin(), temp.end(), temp.begin(), [](char x) {
			return toupper(x, locale());
		});

	for (auto element : EinsShell::commands) {
			string transfelement = element.cmd;

			transform(transfelement.begin(), transfelement.end(), transfelement.begin(), [](char x) {
				return toupper(x, locale());
			});

			string shortedElement =  transfelement.substr(0, temp.length());

			if (transfelement == temp) {
				*bold = 1;
				*color = 35;
				return strdup((" " + element.hints).c_str()); //show hints if full word

			} else if (transfelement + " " == temp) {
				*bold = 1;
				*color = 35;
				return strdup((element.hints).c_str()); //show hints if complete word + space

			} else if (shortedElement == temp) { //TODO: consider remove, seems to be confusing
				*bold = 0;
				*color = 34;
				return strdup(element.cmd.substr(temp.length(), element.cmd.length()).c_str()); //complete to the closest word
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

EinsShell::EinsShell(shared_ptr<Einstein> eins)
{
	this->eins  		= eins;
	this->prompt		= "Einstein> ";
	this->historyPath	= string(getenv("HOME")) + "/.einstein.hist";
	this->historyLength	= 500;
	this->continueShell	= true;

	this->commands = ShellCommand::getCommandList();

	linenoiseHistorySetMaxLen(this->historyLength);
	linenoiseHistoryLoad(this->historyPath.c_str());
	linenoiseClearScreen();
	linenoiseSetMultiLine(1);

	linenoiseSetCompletionCallback(completeln);
	linenoiseSetHintsCallback(hintsln);
	linenoiseSetFreeHintsCallback(hintsFree);

	ShellCommand::eins = eins;
}

EinsShell::~EinsShell()
{
	linenoiseHistorySetMaxLen(1);
	linenoiseHistorySetMaxLen(0);
}

int EinsShell::startShell()
{
	int ret = 0;

	while (this->continueShell) {
		char *tmp = linenoise(this->prompt.c_str());

		if (tmp != NULL && tmp[0] != 0) {
			linenoiseHistoryAdd(tmp);

			if (linenoiseHistorySave(this->historyPath.c_str())) {
				perror("WTF");
				cerr << "Cant save history to: " << this->historyPath.c_str() << endl;
			}

			int result = this->executeCmd(string(tmp));

			if (result == 1) {
				ret = 0;
				this->continueShell = false;

			} else if (result) {
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

void EinsShell::waitForEinstein()
{
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

		this->eins->mutex_lock();
		int ret = found->exec(cmd);
		this->eins->mutex_unlock();

		return ret;

	} else {
		cout << "Command \"" << cmd << "\" not available" << endl;
	}

	return 0;
}
