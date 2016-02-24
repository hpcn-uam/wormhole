#include <einstein.hpp>


Eins2WormConn::Eins2WormConn(uint16_t id, uint16_t listenPort, int16_t core, string ip, string connectionDescription, string host, string programName)
{
	this->ws.id = id;
	this->ws.listenPort = listenPort;
	this->ws.core = core;
	this->ws.IP = inet_addr(ip.c_str());
	this->ws.connectionDescriptionLength = connectionDescription.size();
	this->ws.connectionDescription = static_cast<uint8_t *>(malloc(connectionDescription.size()));
	memcpy(this->ws.connectionDescription, connectionDescription.c_str(), connectionDescription.size());
	this->host = host;
	this->programName = programName;
	this->halting = false;
}

Eins2WormConn::~Eins2WormConn()
{
	free(this->ws.connectionDescription);
	ctrlMsgType msg = HALT;
	cerr << "Enviando HALT al Worm id = " << this->ws.id << endl;

	if (tcp_message_send(this->socket, &msg, sizeof(msg)) != 0) {
		throw std::runtime_error("Error sending HALT");
	}
}

bool EinsConn::keepRunning = true;

Einstein::Einstein(const string configFileName, string listenIp, uint16_t listenPort) : Einstein(configFileName, listenIp, listenPort, true) {}

Einstein::Einstein(const string configFileName, const string listenIp, const uint16_t listenPort, bool autoDeployWorms)
	: ec(listenIp, listenPort, autoDeployWorms)
{

	this->readConfig(configFileName);
	ec.run();
}

Einstein::~Einstein()
{
	cerr << "Eliminando Einstein" << endl;
}

void Einstein::readConfig(const string configFileName)
{

	// TODO: Leer realmente el fichero
	uint16_t id = 1;
	uint16_t baseListenPort = 10000;
	int16_t core = 0;
	string ip = "127.0.0.1";
	//string connectionDescription = "(LISP connection description)";

	FILE *configFile = fopen(configFileName.c_str(), "r");

	if (configFile == 0) {
		throw std::runtime_error("Config file doesn't exist");
	}

	char configLine[4096];
	char programName[4096];
	char host[4096];
	char connectionDescription[4096];

	while (!feof(configFile)) {
		if (fgets(configLine, 4096, configFile) == 0) {
			break;
		}

		int st = sscanf(configLine, "%hu %s %s %hd", &id, programName, host, &core);

		if (st == EOF) {
			break;

		} else if (st != 4) {
			cerr << "Only " << st << "fields were found" << '\n';
			throw std::runtime_error("Bad config file");
		}

		if (fgets(connectionDescription, 4096, configFile) == 0) {
			throw std::runtime_error("Missing worm routing");
		}

		connectionDescription[strlen(connectionDescription) - 1] = 0;

		if (connectionDescription[0] != '\t') {
			throw std::runtime_error("Missing worm routing");
		}

		cerr << "Description: " << connectionDescription + 1 << "|\n";

		unique_ptr<Eins2WormConn> wc(new Eins2WormConn(id, baseListenPort + id, core, ip, string(connectionDescription + 1), string(host), string(programName)));

		this->ec.createWorm(std::move(wc), ip);

	}

	cerr << "Launched all worms\n";

	fclose(configFile);
}

EinsConn::EinsConn(string listenIp, uint16_t listenPort) : EinsConn(listenIp, listenPort, true) {}

EinsConn::EinsConn(const string listenIp, const uint16_t listenPort, bool autoDeployWorms)
{
	// Start socket to receive connections from worms
	this->listenIpStr = listenIp;
	this->listenIp = inet_addr(listenIp.c_str());
	this->listenPort = listenPort;
	this->listeningSocket = tcp_listen_on_port(listenPort);

	if (this->listeningSocket == -1) {
		throw std::runtime_error("Error listening to socket");
	}

	this->wormSockets = 0;
	this->fdinfo = 0;
	this->numWormSockets = 0;
	this->previousPollIndex = 0;
	this->autoDeployWorms = autoDeployWorms;

	signal((int) SIGINT, EinsConn::signal_callback_handler);
}

void EinsConn::signal_callback_handler(int signum)
{
	keepRunning = false;
}

EinsConn::~EinsConn()
{
	this->deleteAllWorms();
	close(this->listeningSocket);

	if (this->wormSockets != 0) {
		for (size_t i = 0; i < this->connections.size(); i++) {
			close((this->wormSockets) [i]);
		}

		free(this->wormSockets);
	}

	if (this->fdinfo != 0) {
		free(this->fdinfo);
	}
}

void EinsConn::createWorm(unique_ptr<Eins2WormConn> wc, const string ip)
{
	// TODO: Conectarse al remoto y crear worm

	this->connections.insert(make_pair(wc->ws.id, std::move(wc)));
	this->numWormSockets = this->connections.size();
	void *ret = realloc(static_cast<void *>(wormSockets), this->numWormSockets * sizeof(int));

	if (ret == 0) {
		throw std::runtime_error("Error reallocating socket array");
	}

	bzero(ret, sizeof(int) * this->numWormSockets);
	this->wormSockets = static_cast<int *>(ret);


	ret = realloc(static_cast<void *>(fdinfo), this->connections.size() * sizeof(struct pollfd));

	if (ret == 0) {
		throw std::runtime_error("Error reallocating poll array");
	}

	bzero(ret, sizeof(struct pollfd) * this->numWormSockets);
	this->fdinfo = static_cast<struct pollfd *>(ret);

}


void EinsConn::deleteWorm(const uint16_t id)
{
	cerr << "Deleting worm id = " << id << endl;

	this->connections.erase(id);
}

void EinsConn::deleteAllWorms()
{
	cerr << "Deleting all worms" << endl;

	while (!this->connections.empty()) {
		this->deleteWorm(this->connections.rbegin()->first);
	}

	cerr << "Done" << endl;
}

void EinsConn::run()
{
	// Deploy worms
	if (this->autoDeployWorms) {
		for (auto connIterator = this->connections.begin(); connIterator != this->connections.end(); connIterator++) {
			deployWorm(*(connIterator->second));
		}
	}

	// Wait for connections from worms
	for (size_t i = 0; i < this->connections.size(); i++) {
		if (setupWorm() != 0) {
			i--;
		}
	}

	cerr << "Completed setup of all worms\n";

	for (; keepRunning;) {
		pollWorms();
	}

	throw std::runtime_error("Forcing to delete Einstein");
}

int EinsConn::setupWorm()
{
	int currentWormSocket = tcp_accept(this->listeningSocket, NULL);

	if (currentWormSocket == -1) {
		throw std::runtime_error("Error accepting connection");
	}

	// Get hello message
	size_t hellomsgSize = sizeof(enum ctrlMsgType) + sizeof(uint16_t);
	uint8_t hellomsg[hellomsgSize];

	if (tcp_message_recv(currentWormSocket, hellomsg, hellomsgSize, 1) != hellomsgSize) {
		throw std::runtime_error("Error receiving message");
	}

	enum ctrlMsgType *msgType = reinterpret_cast<enum ctrlMsgType *>(hellomsg);

	if (*msgType != HELLOEINSTEIN) {
		return 1;
	}

	uint16_t wormId = ntohs(* ((uint16_t *)(hellomsg + sizeof(enum ctrlMsgType))));
	connectWorm(wormId, currentWormSocket);

	// Send configuration message
	const void *wormSetup = static_cast<const void *>(& (this->connections.at(wormId)->ws));

	if (tcp_message_send(currentWormSocket, wormSetup, sizeof(WormSetup)) != 0) {
		throw std::runtime_error("Error sending message");
	}

	const void *connDescription = static_cast<const void *>(this->connections.at(wormId)->ws.connectionDescription);

	if (tcp_message_send(currentWormSocket, connDescription, this->connections.at(wormId)->ws.connectionDescriptionLength) != 0) {
		throw std::runtime_error("Error sending message");
	}

	cerr << "Completed setup of worm " << wormId << '\n';
	return 0;
}

void EinsConn::connectWorm(const uint16_t id, const int socket)
{
	this->connections.at(id)->socket = socket;

	// Add socket to the list used for polling
	int socketIndex = distance(this->connections.begin() , this->connections.find(id));
	wormSockets[socketIndex] = socket;

	// TODO: Insert socket descriptor in property wormSockets
}

void EinsConn::deployWorm(Eins2WormConn &wc)
{
	char executable[4096];
	sprintf(executable, "scp %s.tgz %s:~", wc.programName.c_str(), wc.host.c_str());

	if (system(executable)) {
		cerr << "error executing comand..." << endl;
	}

	sprintf(executable, "ssh -T %s 'tar -xzf %s.tgz; export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:%s/lib;"
			"export WORM_ID=%hu;"
			"export EINSTEIN_PORT=%hu;"
			"export EINSTEIN_IP=%s;"
			"nohup sh %s/run.sh > /dev/null 2>&1 &'",
			wc.host.c_str(), wc.programName.c_str(), wc.programName.c_str(), wc.ws.id, this->listenPort,
			this->listenIpStr.c_str(), wc.programName.c_str());

	if (system(executable)) {
		cerr << "error executing comand..." << endl;
	}
}

void EinsConn::pollWorms()
{

	int i = 0, j = 0;

	for (i = 0, j = 0; i < this->numWormSockets; ++i, ++j) {
		memset(& (this->fdinfo[i]), 0, sizeof(struct pollfd));

		if (this->wormSockets[j] == -1) { //TODO documentar que hace este bucle for...
			// Relaunch worm
			auto connIterator = connections.begin();

			for (int k = 0; k < j; ++k) {
				connIterator++;
			}

			//deployWorm(*(connIterator->second));
			//setupWorm();
		}

		this->fdinfo[i].fd = this->wormSockets[j];
		this->fdinfo[i].events = POLLIN | POLLHUP | POLLRDNORM | POLLNVAL;
	}

	this->numFilledPolls = i;

	int st;
	st = poll(this->fdinfo, this->numFilledPolls, 1);

	if (st == -1) {
		if (keepRunning) {
			throw std::runtime_error("Failed poll");

		} else {
			return;
		}

	} else if (st) {
		// Check all sockets from the socket next to the one that received data in the previous iteration
		for (int i = this->previousPollIndex + 1, count = 0; count < this->numFilledPolls; ++i, count++) {
			if (i == this->numFilledPolls) {
				i = 0;
			}

			if (this->wormSockets[i] == -1) {
				// TODO: Try to reconnect. If it doesn't work launch worm again
			}

			if (this->fdinfo[i].revents & POLLIN) {
				enum ctrlMsgType ctrlMsg;

				if (tcp_message_recv(this->fdinfo[i].fd, reinterpret_cast<uint8_t *>(&ctrlMsg), sizeof(enum ctrlMsgType), 1) != sizeof(enum ctrlMsgType)) {
					// Closed socket
					this->wormSockets[i] = -1;
					continue;
				}

				// Check message and do corresponding action
				switch (ctrlMsg) {
				case QUERYID:
					// Get worm id
					uint16_t wormId;

					if (tcp_message_recv(this->fdinfo[i].fd, static_cast<void *>(&wormId), sizeof(uint16_t), 1) != sizeof(uint16_t)) {
						// Closed socket
						this->wormSockets[i] = -1;
						continue;
					}

					// Send worm configuration message
					try {

						const void *wormSetup = static_cast<const void *>(& (this->connections.at(wormId)->ws));
						enum ctrlMsgType okMsg = CTRL_OK;

						cerr << "Received request for information of worm " << wormId << endl;

						if (tcp_message_send(this->fdinfo[i].fd, reinterpret_cast<uint8_t *>(&okMsg), sizeof(enum ctrlMsgType)) != 0) {
							// Closed socket
							this->wormSockets[i] = -1;
							continue;
						}

						if (tcp_message_send(this->fdinfo[i].fd, wormSetup, sizeof(WormSetup)) != 0) {
							// Closed socket
							this->wormSockets[i] = -1;
							continue;
						}

					} catch (std::out_of_range &e) {
						// Send error
						enum ctrlMsgType errorMsg = CTRL_ERROR;
						cerr << "Worm " << wormId << " does not exist" << endl;

						if (tcp_message_send(this->fdinfo[i].fd, reinterpret_cast<uint8_t *>(&errorMsg), sizeof(enum ctrlMsgType)) != 0) {
							// Closed socket
							this->wormSockets[i] = -1;
							continue;
						}
					}

					break;

				case HALT: { // TODO better implementation
					for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
						if (it->second->socket == this->wormSockets[i]) {
							it->second->halting = true;
							cerr << "El worm " << it->first << " ha finalizado su tarea." << endl;
						}
					}

					uint8_t flag = 1;

					for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
						if (!it->second->halting) {
							flag = 0;
							break;
						}
					}

					if (flag) {
						cerr << "Cerrando Einstein debido a que todas las tareas han sido completadas" << endl;
						this->deleteAllWorms();
						exit(0); //TODO cambiar por un cierre mas ordenado, como por ejemplo, modificando la variable de salida utilizada para el cntl+c
					}

					break;
				}

				case DOWNLINK:

					// TODO
				case OVERLOAD:

					// TODO
				case UNDERLOAD:

					// TODO
				default:
					// Send error
					enum ctrlMsgType errorMsg = CTRL_ERROR;

					if (tcp_message_send(this->fdinfo[i].fd, reinterpret_cast<uint8_t *>(&errorMsg), sizeof(enum ctrlMsgType)) != 0) {
						// Closed socket
						this->wormSockets[i] = -1;
						continue;
					}
				}



			} else if (this->fdinfo[i].revents & POLLHUP || this->fdinfo[i].revents & POLLRDNORM || this->fdinfo[i].revents & POLLNVAL) {
				this->wormSockets[i] = -1;
			}
		}

	} else {
		for (int i = 0; i < this->numFilledPolls; ++i) {
			if (this->wormSockets[i] == -1) {
				// TODO: Try to reconnect. If it doesn't work launch worm again

			}
		}
	}


}
