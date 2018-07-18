#include <iomanip>
#include <ios>
#include <sstream>

#include <wh_config.h>
#include <wh_version.h>
#include <zeus/shellcommand.hpp>

using namespace zeus;

shared_ptr<Zeus> ShellCommand::zeus;

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

string ShellCommand::normalize(string str, int length)
{
	string tmp = str.substr(0, length);
	return normalize(tmp);
}

int ShellCommand::forHole(string cmd, function<int(shared_ptr<Hole>, string)> fn)
{
	// all holes
	int ret = 0;

	auto pos = normalize(cmd).find(normalize("all"));

	if (pos != string::npos) {
		string param;

		if (pos + 4 <= cmd.length()) {
			param = cmd.substr(pos + 4, cmd.length());

		} else {
			param = "";
		}

		for (auto elem : zeus->ec.connections) {
			ret += fn(elem.second, param);
		}

	} else {
		try {
			auto elem    = zeus->ec.connections.at(stoi(cmd.substr(cmd.find(' ') + 1, cmd.length())));
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
			cout << "Hole with ID=\"" << cmd.substr(cmd.find(' ') + 1, cmd.length()) << "\" does not exists." << endl;
		}
	}

	return ret;
}

bool ShellCommand::operator<(const ShellCommand& rhs) const
{
	int la  = this->cmd.length();
	int lb  = rhs.cmd.length();
	int len = la > lb ? lb : la;

	string a = normalize(this->cmd, len);
	string b = normalize(rhs.cmd, len);

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
	                        "Stops all holes and closes zeus",
	                        "Stops all holes sending a special HALT message and closes zeus"));

	ret.insert(ShellCommand("quit",
	                        cmdQuit,
	                        "Stops all holes and then closes zeus",
	                        "Send HALT to all holes and then exites Zeus. At this time this command is equivalent to "
	                        "HALT, in future, halt wont close zeus"));

	ret.insert(ShellCommand("list",
	                        cmdList,
	                        "Lists information about many things",
	                        "Lists information about many internal thigs, such as holes, etc.",
	                        "[holes]"));

	ret.insert(ShellCommand(
	    "ping", cmdPing, "Pings a hole", "Send a Ping command to a hole, waiting for a pong response.", "<hole id/all>"));

	ret.insert(ShellCommand(
	    "kill",
	    cmdKill,
	    "Kills a hole",
	    "Send a HALT command to a hole. The hole would not be automatically rzeustantiated anymore (usefull to debug)",
	    "<hole id/all>"));

	ret.insert(ShellCommand("chRoute",
	                        cmdChRoute,
	                        "Changes the route of one or all holes",
	                        "Changes the route of one or all holes",
	                        "<hole id/all> <new route>"));

	ret.insert(ShellCommand("version", cmdVersion, "Show libraries's version", "Show libraries's version"));

	ret.insert(ShellCommand("statistics",
	                        cmdStatistics,
	                        "Show hole connection statistics",
	                        "Retrieve from specific (or every) connected hole each connection statistics.",
	                        "<hole id/all>"));

	return ret;
}

int ShellCommand::cmdHelp(string cmd)
{
	auto commandList = ShellCommand::getCommandList();

	// No second parameter
	if (cmd.length() < 6 || cmd.find(' ') == string::npos) {
		cout << "Welcome to Zeus!" << endl;
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

	cout << "Halting Zeus and all holes..." << endl;
	zeus->ec.keepRunning = false;

	return 1;
}

int ShellCommand::cmdQuit(string cmd)
{
	UNUSED(cmd);

	cout << "Exiting zeus..." << endl;
	zeus->ec.keepRunning = false;
	ShellCommand::cmdHalt(cmd);

	return 1;  // for efectively halt
}

int ShellCommand::cmdList(string cmd)
{
	if (normalize(cmd).find(normalize("holes")) != string::npos) {
		cout << "Listing all holes:" << endl;

		for (auto elem : zeus->ec.connections) {
			cout << *elem.second << endl;
		}

	} else {
		cout << "Cant list that." << endl;
	}

	return 0;
}

int ShellCommand::cmdPing(string cmd)
{
	return forHole(cmd, [](shared_ptr<Hole> elem, string param) -> int {
		UNUSED(param);

		cout << "Pinging Hole ID=" << elem->ws.id << "..." << flush;
		int64_t us = elem->ping();

		if (us > 0)
			cout << "Pong! " << us / 1000 << "," << us - (us / 1000) * 1000 << " us" << endl;
		else {
			cout << "Timeout! " << us / -1000 << "," << 0 - (us - (us / 1000) * 1000) << " us" << endl;
		}
		return 0;
	});
}

int ShellCommand::cmdKill(string cmd)
{
	return forHole(cmd, [](shared_ptr<Hole> elem, string param) -> int {
		UNUSED(param);

		cout << "Killing Hole ID=" << elem->ws.id << "..." << flush;
		int64_t us = elem->kill();

		if (us)
			cout << "Survived!" << endl;
		else
			cout << "Killed!" << endl;

		return 0;
	});
}

int ShellCommand::cmdChRoute(string cmd)
{
	return forHole(cmd, [](shared_ptr<Hole> elem, string param) -> int {

		cout << "Changing hole " << elem->ws.id << " route into: \"" << Hole::expandCDescription(param) << "\"" << endl;

		elem->chroute(param);

		return 0;
	});
}

int ShellCommand::cmdVersion(string cmd)
{
	UNUSED(cmd);
	cout << "Libhole version: " << wh_VERSION << endl;
	cout << "Netlib version: " << netlib_VERSION << endl;
	cout << "Hptl version: " << hptl_VERSION << endl;
#ifdef WH_SSL
	cout << "SSL version: " << LIBRESSL_VERSION_TEXT << endl;
#endif

	return 0;
}

string humanReadableSpeed(double speed /*in bits*/)
{
	int i               = 0;
	const char* units[] = {"bps", "Kbps", "Mbps", "Gbps", "Tbps", "Pbps", "Ebps", "Zbps", "Ybps"};
	while (speed > 1000) {
		speed /= 1000;
		i++;
	}
	stringstream sb;
	sb << std::fixed << std::setprecision(2) << speed << units[i];
	return sb.str();
}

string humanReadableSize(double size /*in bytes*/)
{
	int i               = 0;
	const char* units[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
	while (size > 1024) {
		size /= 1024;
		i++;
	}
	stringstream sb;
	sb << std::fixed << std::setprecision(2) << size << units[i];
	return sb.str();
}

int ShellCommand::cmdStatistics(string cmd)
{
#ifdef WH_STATISTICS

	return forHole(cmd, [](shared_ptr<Hole> elem, string param) -> int {
		vector<ConnectionStatistics> stats;
		ConnectionStatistics total;

		cout << "== Hole " << elem->ws.id << " ==" << endl;
		cout << "Input: " << endl;
		stats = elem->getStatistics(true);
		total = {0};
		for (ConnectionStatistics stat : stats) {
			cout << "\t" << stat.holeId << ": ";
			cout << humanReadableSize(stat.lastMinIO_tmp) << " in current minute \t";
			cout << humanReadableSpeed(stat.lastMinIO * 8 / 60) << " last minute \t";
			cout << humanReadableSize(stat.totalIO) << " in total.\t";
			cout << endl;
			total.lastMinIO += stat.lastMinIO;
		}
		if (stats.size() > 1)
			cout << "\t\tTotal last minute:" << humanReadableSpeed(total.lastMinIO * 8 / 60) << endl;

		cout << "Output: " << endl;
		stats = elem->getStatistics(false);
		total = {0};
		for (ConnectionStatistics stat : stats) {
			cout << "\t" << stat.holeId << ": ";
			cout << humanReadableSize(stat.lastMinIO_tmp) << " in current minute \t";
			cout << humanReadableSpeed(stat.lastMinIO * 8 / 60) << " last minute \t";
			cout << humanReadableSize(stat.totalIO) << " in total.\t";
			cout << endl;
			total.lastMinIO += stat.lastMinIO;
		}
		if (stats.size() > 1)
			cout << "\t\tTotal last minute:" << humanReadableSpeed(total.lastMinIO * 8 / 60) << endl;

		return 0;
	});

#else  // Not compiled statistics
	cout << "Function not compiled, to get statistics compile with WH_STATISTICS set to On " << endl;
#endif
}