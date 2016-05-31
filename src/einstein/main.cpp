#include <einstein/einstein.hpp>
#include <einstein/einsshell.hpp>

#include <cassert>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace einstein;

int main(int argc, char **argv)
{
	shared_ptr<Einstein> eins;

	if (argc == 4) {
		try {
			vector<string> runParams;
			runParams.push_back(argv[3]);
			eins = make_shared<Einstein>(argv[1], argv[2], 5000, true, runParams);

		} catch (exception &e) {
			std::cerr << "Exception: " << e.what() << '\n';
			return 1;
		}

	} else if (argc == 3) {
		try {
			eins = make_shared<Einstein>(argv[1], argv[2], 5000, true);

		} catch (exception &e) {
			std::cerr << "Exception: " << e.what() << '\n';
			return 1;
		}

	} else {
		std::cerr << "No pararms provided" << endl;
		std::cerr << "Try: " << argv[0] << "<CONFIG FILE> <IP> [More Params]" << endl;
		return 1;
	}

	eins->openThreadedHoles();

	unique_ptr<EinsShell> cmd(new EinsShell(eins));
	int ret = cmd->startShell();

	if (ret)
		std::cerr << "Some error happened...!" << endl
				  << "Closing Einstein & Halting Everything"  << endl;

	return ret;
}
