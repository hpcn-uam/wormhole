#include <einstein/einstein.hpp>

using namespace einstein;

Einstein::Einstein(const string configFileName, string listenIp, uint16_t listenPort)
	: Einstein(configFileName, listenIp, listenPort, true) {}

Einstein::Einstein(const string configFileName, const string listenIp, const uint16_t listenPort, bool autoDeployWorms)
	: Einstein(configFileName, listenIp, listenPort, autoDeployWorms, vector<string>()) {}

Einstein::Einstein(const string configFileName, string listenIp, uint16_t listenPort, bool autoDeployWorms, vector<string> runParams)
	: ec(listenIp, listenPort, autoDeployWorms, runParams)
{

	this->readConfig(configFileName);
}

Einstein::~Einstein()
{
	cerr << "Eliminando Einstein" << endl;
	this->ec.keepRunning = false;

	if (this->thr.joinable()) {
		this->thr.join();
	}
}

void Einstein::openHoles()
{
	ec.run();
}

void Einstein::openThreadedHoles()
{
	this->thr = thread([this] {this->openHoles();}); //Lambda
}

void Einstein::readConfig(const string configFileName)
{

	// TODO: Leer realmente el fichero
	uint16_t id = 0;
	uint16_t baseListenPort = 10000;
	int64_t core = 0;
	//string connectionDescription = "(LISP connection description)";

	FILE *configFile = fopen(configFileName.c_str(), "r");

	if (configFile == 0) {
		throw std::runtime_error("Config file doesn't exist");
	}

	char id_string[128]; //TODO FIX POSIBLE OVERFLOW
	char configLine[4096]; //TODO FIX POSIBLE OVERFLOW
	char programName[512]; //TODO FIX POSIBLE OVERFLOW
	char host[512]; //TODO FIX POSIBLE OVERFLOW
	char connectionDescription[4096]; //TODO FIX POSIBLE OVERFLOW

	bool createAnotherWorm = false;

	while (!feof(configFile)) {
		if (fgets(configLine, 4096, configFile) == 0) {
			break;
		}

		int st = sscanf(configLine, "%s %s %s %lx", id_string, programName, host, &core);

		if (st == EOF) {
			break;

		} else if (st < 4) {
			cerr << "Only " << st << "fields were found" << endl;
			throw std::runtime_error("Bad config file");
		}

		if (fgets(connectionDescription, 4096, configFile) == 0) {
			throw std::runtime_error("Missing worm routing");
		}

		connectionDescription[strlen(connectionDescription) - 1] = 0;

		do {
			int firstId, lastId;

			if (string(id_string).find("-") != string::npos) {
				firstId = atoi(strtok(id_string, "-"));
				lastId  = atoi(strtok(NULL, "-"));

				if (firstId >= lastId) {
					throw std::runtime_error("Non valid id range: \"" + string(id_string) + "\"");
				}

				id = firstId;
				createAnotherWorm = true;

			} else if (createAnotherWorm) {
				if (id >= firstId && id < (lastId - 1)) {
					id++;
					createAnotherWorm = true;

				} else {
					id++;
					createAnotherWorm = false;
				}

			} else {
				id = atoi(id_string);
			}

			cerr << "[" << id << "] " << host << " " << programName << endl;

			if (connectionDescription[0] != '\t') {
				throw std::runtime_error("Missing worm routing");
			}

			cerr << "Description: |" << connectionDescription + 1 << "|" << endl;

			//check if ipaddr or name
			struct sockaddr_in sa;
			int result = inet_pton(AF_INET, host, &(sa.sin_addr));
			string ip;

			if (result == -1 || result == 0) {
				struct hostent *tmp = gethostbyname(host);

				if (tmp == NULL) {
					throw std::runtime_error("Host '" + string(host) + "' does not exists");
				}

				if (tmp->h_addr_list[0] == NULL) {
					throw std::runtime_error("Host '" + string(host) + "' does not have a valid IP");
				}

				ip = string(inet_ntoa(*((struct in_addr *)tmp->h_addr_list[0])));

			} else {
				ip = string(host);
			}

			unique_ptr<Worm> wc(new Worm(id, baseListenPort + id, core, ip, string(connectionDescription + 1), string(host), string(programName)));

			/*Check for advanced options*/

			/*SSL*/
			if (string(configLine).find("SSL") != string::npos) {
				wc->ws.isSSLNode = 1;
			}

			this->ec.createWorm(std::move(wc), ip);
		} while (createAnotherWorm);
	}

	cerr << "Launched all worms" << endl;

	fclose(configFile);
}
