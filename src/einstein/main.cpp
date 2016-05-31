#include <einstein/einstein.hpp>

#include <cassert>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace einstein;

int main(int argc, char **argv)
{
	Einstein *eins;

	if (argc == 4) {
		try {
			vector<string> runParams;
			runParams.push_back(argv[3]);
			eins = new Einstein(argv[1], argv[2], 5000, true, runParams);
			std::cout << "Éxito\n";

		} catch (exception &e) {
			std::cerr << "Excepción: " << e.what() << '\n';
			return 1;
		}

	} else if (argc == 3) {
		try {
			eins = new Einstein(argv[1], argv[2], 5000, true);
			std::cout << "Éxito\n";

		} catch (exception &e) {
			std::cerr << "Excepción: " << e.what() << '\n';
			return 1;
		}

	} else {
		std::cerr << "No pararms provided" << endl;
		return 1;
	}

	delete eins;
	return 0;
}
