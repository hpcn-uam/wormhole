#include <einstein/einstein.hpp>

using namespace einstein;

Einstein::Einstein(const string configFileName, string listenIp, uint16_t listenPort)
	: Einstein(configFileName, listenIp, listenPort, true) {}

Einstein::Einstein(const string configFileName, const string listenIp, const uint16_t listenPort, bool autoDeployWorms)
	: ec(listenIp, listenPort, autoDeployWorms)
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

	while (!feof(configFile)) {
		bool createAnotherWorm = false;

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

			cerr << "\tDescription: |" << Worm::expandCDescription(string(connectionDescription)) << "|" << endl;

			vector<string> runParams;

			unique_ptr<Worm> wc(new Worm(id, baseListenPort + id, core, string(connectionDescription), string(host), string(programName), runParams));

			/*Check for advanced options*/
			string sconfline = string(configLine);
			sconfline = sconfline.substr(0, sconfline.find('#'));

			/*SSL*/
			if (sconfline.find("SSL") != string::npos) {
				wc->ws.isSSLNode = 1;
			}

			/*PARAMS*/

			while (sconfline.find("PARAM=") != string::npos) {
				auto pos = sconfline.find("PARAM=");
				sconfline = sconfline.substr(pos + 6);
				pos = sconfline.find_first_of(" \n\r\t");

				if (pos != string::npos) {
					wc->runParams.push_back(sconfline.substr(0, pos));

				} else {
					wc->runParams.push_back(sconfline);
				}
			}

			if (wc->runParams.size() > 0) {
				cerr << "\tParams:";

			for (auto param: wc->runParams) {
					cerr << " " << param;
				}

				cerr << endl;
			}

			this->ec.createWorm(std::move(wc));
		} while (createAnotherWorm);
	}

	cerr << "Launched all worms" << endl;

	fclose(configFile);
}

void Einstein::mutex_lock()
{
	ec.mutex_lock();
}
void Einstein::mutex_unlock()
{
	ec.mutex_unlock();
}
