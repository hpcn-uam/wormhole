#include <einstein/shellcommand.hpp>

using namespace einstein;

shared_ptr<Einstein> ShellCommand::eins;

ShellCommand::ShellCommand(string cmd)
{
	this->cmd  		= cmd;
}

ShellCommand::ShellCommand(
	string cmd,
	function<int(string)> exec,
	string shortHelp,
	string longHelp)
	: ShellCommand::ShellCommand(cmd, exec, shortHelp, longHelp, "") {};

ShellCommand::ShellCommand(
	string cmd,
	function<int(string)> exec,
	string shortHelp,
	string longHelp,
	string hints)
{
	this->cmd  		= cmd;
	this->exec		= exec;

	this->shortHelp	= shortHelp;
	this->longHelp	= longHelp;
	this->hints		= hints;
}

ShellCommand::~ShellCommand()
{

}

string ShellCommand::normalize(string str)
{
	string out = str;
	transform(out.begin(), out.end(), out.begin(), [](char x) {
		return toupper(x, locale());
	});
	return out;
}

bool ShellCommand::operator< (const ShellCommand &rhs) const
{
	string a = normalize(this->cmd);
	string b = normalize(rhs.cmd);

	return a < b;
	//return this->cmd < rhs.cmd;
}

int ShellCommand::execute(string cmd)
{
	return this->exec(cmd);
}

set<ShellCommand> ShellCommand::getCommandList()
{
	set<ShellCommand> ret = set<ShellCommand>();

	ret.insert(ShellCommand(
		"help",
		cmdHelp,
		"Prints this menu",
		"Shows information about commands or the general usage of this application",
		"[Command to show information]"));

	ret.insert(ShellCommand(
		"halt",
		cmdHalt,
		"Stops all worms and closes einstein",
		"Stops all worms sending a special HALT message and closes einstein"));

	ret.insert(ShellCommand(
		"list",
		cmdList,
		"Lists information about many things",
		"Lists information about many internal thigs, such as worms, etc.",
		"[worms]"));


	return ret;
}

int ShellCommand::cmdHelp(string cmd)
{
	auto commandList = ShellCommand::getCommandList();

	// No second parameter
	if (cmd.length() < 6 && cmd.find(' ') == string::npos) {
		cout << "Welcome to Einstein!" << endl;
		cout << endl;
		cout << "Available commands are:" << endl;

	for (auto cmd: commandList) {
			cout << "\t" << cmd.cmd << ": \t" << cmd.shortHelp << endl;
		}


	} else {
		string firstcmd  = cmd.substr(0, cmd.find(' '));
		string secondcmd = cmd.substr(cmd.find(' ') + 1, cmd.length());
		auto cmd = commandList.find(ShellCommand(secondcmd));

		if (cmd != commandList.end()) {
			cout << "Help for command " << cmd->cmd << ": \t" << cmd->longHelp << endl;

		} else {
			cout << "The command \"" << secondcmd << "\" is not available in this version" << endl;
		}
	}

	return 0;
}

int ShellCommand::cmdHalt(string cmd)
{
	UNUSED(cmd);

	cout << "Halting Einstein and all worms..." << endl;
	eins->ec.keepRunning = false;

	return 1;
}

int ShellCommand::cmdList(string cmd)
{
	if (normalize(cmd).find(normalize("worms")) != string::npos) {
		cout << "Listing all worms:" << endl;

	for (auto elem : eins->ec.connections) {
			cout << *elem.second << endl;
		}
	}

	return 0;
}