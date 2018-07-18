#include <zeus/zeus.hpp>

using namespace zeus;

Zeus::Zeus(const string configFileName, string listenIp, uint16_t listenPort)
    : Zeus(configFileName, listenIp, listenPort, true)
{
}

Zeus::Zeus(const string configFileName, const string listenIp, const uint16_t listenPort, bool autoDeployHoles)
    : ec(listenIp, listenPort, autoDeployHoles)
{
	this->readConfig(configFileName);
}

Zeus::~Zeus()
{
	cerr << "Deleting Zeus..." << endl;
	this->ec.keepRunning = false;

	if (this->thr.joinable()) {
		this->thr.join();
	}
}

void Zeus::openHoles()
{
	ec.run();
}

void Zeus::openThreadedHoles()
{
	this->thr = thread([this] { this->openHoles(); });  // Lambda
}

void Zeus::readConfig(const string configFileName)
{
	// TODO: Leer realmente el fichero
	uint16_t id             = 0;
	uint16_t baseListenPort = 10000;
	int64_t core            = 0;
	// string connectionDescription = "(LISP connection description)";

	FILE *configFile = fopen(configFileName.c_str(), "r");

	if (configFile == 0) {
		throw std::runtime_error("Config file doesn't exist");
	}

	char id_string[128];               // TODO FIX POSIBLE OVERFLOW
	char configLine[4096];             // TODO FIX POSIBLE OVERFLOW
	char programName[512];             // TODO FIX POSIBLE OVERFLOW
	char host[512];                    // TODO FIX POSIBLE OVERFLOW
	char connectionDescription[4096];  // TODO FIX POSIBLE OVERFLOW

	while (!feof(configFile)) {
		bool createAnotherHole;

		if (fgets(configLine, 4096, configFile) == 0) {
			break;
		}

		int st = sscanf(configLine, "%s %s %s %lx", id_string, programName, host, &core);

		if (st == EOF) {
			break;

		} else if (st < 4) {
			cerr << "Only " << st << " fields were found" << endl;
			throw std::runtime_error("Bad config file");
		}

		if (fgets(connectionDescription, 4096, configFile) == NULL) {
			connectionDescription[0] = '\0';
		}

		connectionDescription[strlen(connectionDescription) - 1] = 0;

		createAnotherHole = false;

		do {
			int firstId = 0, lastId = 0;

			if (string(id_string).find("-") != string::npos) {
				firstId = atoi(strtok(id_string, "-"));
				lastId  = atoi(strtok(NULL, "-"));

				if (firstId >= lastId) {
					throw std::runtime_error("Non valid id range: \"" + string(id_string) + "\"");
				}

				id                = firstId;
				createAnotherHole = true;

			} else if (createAnotherHole) {
				if (id >= firstId && id < (lastId - 1)) {
					id++;
					createAnotherHole = true;

				} else {
					id++;
					createAnotherHole = false;
				}

			} else {
				id = atoi(id_string);
			}

			cerr << "[" << id << "] " << host << " " << programName;

			if (core > 0) {
				cerr << " Core mask: 0x" << hex << core << dec;
			}

			cerr << endl;

			string conndesc = string(connectionDescription);

			if (connectionDescription[0] != '\t') {
				fseek(configFile, -1 - conndesc.length(), SEEK_CUR);
				conndesc = "";
			}

			cerr << "\tDescription: |" << Hole::expandCDescription(conndesc) << "|" << endl;

			vector<string> runParams;

			unique_ptr<Hole> wc(
			    new Hole(id, baseListenPort + id, core, conndesc, string(host), string(programName), runParams));

			/*Check for advanced options*/
			string sconfline = string(configLine);
			sconfline        = sconfline.substr(0, sconfline.find('#'));

			/*SSL*/
			if (sconfline.find("SSL") != string::npos) {
				wc->ws.isSSLNode = 1;
#ifndef WH_SSL
				fprintf(stderr, "[WARNING] SSL is disabled by config. SSL connections may fail if not supported in holes\n");
#endif
			}

			/*Manual Deploy*/
			if (sconfline.find("MANUAL") != string::npos) {
				wc->autoDeploy = false;
			}

			/*PARAMS*/
			while (sconfline.find("PARAM=") != string::npos) {
				auto pos  = sconfline.find("PARAM=");
				sconfline = sconfline.substr(pos + 6);
				pos       = sconfline.find_first_of(" \n\r\t");

				if (pos != string::npos) {
					wc->runParams.push_back(sconfline.substr(0, pos));

				} else {
					wc->runParams.push_back(sconfline);
				}
			}

			if (wc->runParams.size() > 0) {
				cerr << "\tParams:";

				for (auto param : wc->runParams) {
					cerr << " " << param;
				}

				cerr << endl;
			}

			this->ec.createHole(std::move(wc));
		} while (createAnotherHole);
	}

	cerr << "Launched all holes" << endl;

	fclose(configFile);
}

void Zeus::mutex_lock()
{
	ec.mutex_lock();
}
void Zeus::mutex_unlock()
{
	ec.mutex_unlock();
}
