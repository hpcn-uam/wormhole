#include <wh_config.h>
#include <einstein/shellcommand.hpp>

using namespace einstein;

shared_ptr<Einstein> ShellCommand::eins;

ShellCommand::ShellCommand(string cmd)
{
	this->cmd = cmd;
}

ShellCommand::ShellCommand(string cmd, function<int(string)> exec, string shortHelp, string longHelp)
    : ShellCommand::ShellCommand(cmd, exec, shortHelp, longHelp, ""){};

ShellCommand::ShellCommand(string cmd, function<int(string)> exec, string shortHelp, string longHelp, string hints)
{
	this->cmd  = cmd;
	this->exec = exec;

	this->shortHelp = shortHelp;
	this->longHelp  = longHelp;
	this->hints     = hints;
}

ShellCommand::~ShellCommand()
{
}

string ShellCommand::normalize(string str)
{
	string out = str;
	transform(out.begin(), out.end(), out.begin(), [](char x) { return toupper(x, locale()); });
	return out;
}

int ShellCommand::forWorm(string cmd, function<int(shared_ptr<Worm>, string)> fn)
{
	// all worms
	int ret = 0;

	auto pos = normalize(cmd).find(normalize("all"));

	if (pos != string::npos) {
		string param;

		if (pos + 4 <= cmd.length()) {
			param = cmd.substr(pos + 4, cmd.length());

		} else {
			param = "";
		}

		for (auto elem : eins->ec.connections) {
			ret += fn(elem.second, param);
		}

	} else {
		try {
			auto elem    = eins->ec.connections.at(stoi(cmd.substr(cmd.find(' ') + 1, cmd.length())));
			string param = cmd.substr(cmd.find(' ') + 1, cmd.length());

			auto parampos = param.find(' ');

			if (parampos != string::npos) {
				param = param.substr(parampos, param.length());

				if (param.length() > 1) {
					param = param.substr(1, param.length());

				} else {
					param = "";
				}

			} else {
				param = "";
			}

			ret += fn(elem, param);

		} catch (exception) {
			cout << "Worm with ID=\"" << cmd.substr(cmd.find(' ') + 1, cmd.length()) << "\" does not exists." << endl;
		}
	}

	return ret;
}

bool ShellCommand::operator<(const ShellCommand &rhs) const
{
	string a = normalize(this->cmd);
	string b = normalize(rhs.cmd);

	return a < b;
	// return this->cmd < rhs.cmd;
}

int ShellCommand::execute(string cmd)
{
	return this->exec(cmd);
}

set<ShellCommand> ShellCommand::getCommandList()
{
	set<ShellCommand> ret = set<ShellCommand>();

	ret.insert(ShellCommand("help",
	                        cmdHelp,
	                        "Prints this menu",
	                        "Shows information about commands or the general usage of this application",
	                        "[Command to show information]"));

	ret.insert(ShellCommand("halt",
	                        cmdHalt,
	                        "Stops all worms and closes einstein",
	                        "Stops all worms sending a special HALT message and closes einstein"));

	ret.insert(ShellCommand("list",
	                        cmdList,
	                        "Lists information about many things",
	                        "Lists information about many internal thigs, such as worms, etc.",
	                        "[worms]"));

	ret.insert(ShellCommand(
	    "ping", cmdPing, "Pings a worm", "Send a Ping command to a worm, waiting for a pong response.", "<worm id/all>"));

	ret.insert(ShellCommand("chRoute",
	                        cmdChRoute,
	                        "Changes the route of one or all worms",
	                        "Changes the route of one or all worms",
	                        "<worm id/all> <new route>"));

	ret.insert(ShellCommand("version", cmdVersion, "Show libraries's version", "Show libraries's version"));

	return ret;
}

int ShellCommand::cmdHelp(string cmd)
{
	auto commandList = ShellCommand::getCommandList();

	// No second parameter
	if (cmd.length() < 6 || cmd.find(' ') == string::npos) {
		cout << "Welcome to Einstein!" << endl;
		cout << endl;
		cout << "Available commands are:" << endl;

		for (auto cmd : commandList) {
			cout << "\t" << cmd.cmd << ": \t" << cmd.shortHelp << endl;
		}

	} else {
		string firstcmd  = cmd.substr(0, cmd.find(' '));
		string secondcmd = cmd.substr(cmd.find(' ') + 1, cmd.length());
		auto cmd         = commandList.find(ShellCommand(secondcmd));

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

	} else {
		cout << "Cant list that." << endl;
	}

	return 0;
}

int ShellCommand::cmdPing(string cmd)
{
	return forWorm(cmd, [](shared_ptr<Worm> elem, string param) -> int {
		UNUSED(param);

		cout << "Pinging Worm ID=" << elem->ws.id << "..." << flush;
		int64_t us = elem->ping();

		if (us > 0)
			cout << "Pong! " << us / 1000 << "," << us - (us / 1000) * 1000 << " us" << endl;
		else {
			cout << "Timeout! " << us / -1000 << "," << 0 - (us - (us / 1000) * 1000) << " us" << endl;
		}
		return 0;
	});
}

int ShellCommand::cmdChRoute(string cmd)
{
	return forWorm(cmd, [](shared_ptr<Worm> elem, string param) -> int {

		cout << "Changing worm " << elem->ws.id << " route into: \"" << Worm::expandCDescription(param) << "\"" << endl;

		elem->chroute(param);

		return 0;
	});
}

int ShellCommand::cmdVersion(string cmd)
{
	UNUSED(cmd);
	cout << "Libworm version: " << wh_VERSION << endl;
	cout << "Netlib version: " << netlib_VERSION << endl;
	cout << "Hptl version: " << hptl_VERSION << endl;
	cout << "SSL version: " << LIBRESSL_VERSION_TEXT << endl;

	return 0;
}