#include <zeus/zeusshell.hpp>
#include <zeus/zeus.hpp>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <cassert>
#include <cstring>

using namespace zeus;

int main(int argc, char **argv)
{
	shared_ptr<Zeus> zeus;

	hptl_init(NULL);
	netlib_init(1, argv);

	if (argc == 3) {
		try {
			zeus = make_shared<Zeus>(argv[1], argv[2], 5000, true);

		} catch (exception &e) {
			std::cerr << "Exception: " << e.what() << '\n';
			return 1;
		}

	} else {
		std::cerr << "No pararms provided" << endl;
		std::cerr << "Try: " << argv[0] << " <CONFIG FILE> <IP>" << endl;
		return 1;
	}

	unique_ptr<ZeusShell> cmd(new ZeusShell(zeus));

	std::cerr << "Launching Zeus, please wait..." << endl;
	zeus->openThreadedHoles();
	cmd->waitForZeus();

	int ret = cmd->startShell();

	if (ret)
		std::cerr << "Some error happened...!" << endl << "Closing Zeus & Halting Everything" << endl;

	return ret;
}
