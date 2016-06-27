#include <einstein/connection.hpp>

using namespace einstein;

bool Connection::keepRunning = true;

Connection::Connection(string listenIp, uint16_t listenPort)
	: Connection(listenIp, listenPort, true) {}

Connection::Connection(const string listenIp, const uint16_t listenPort, bool autoDeployWorms)
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

	signal((int) SIGINT, Connection::signal_callback_handler);
}

void Connection::signal_callback_handler(int signum)
{
	UNUSED(signum);

	keepRunning = false;
}

Connection::~Connection()
{
	if (this->setupThread.joinable()) {
		this->setupThread.join();
	}

	this->deleteAllWorms();
	close(this->listeningSocket);

	if (this->wormSockets != 0) {
		for (size_t i = 0; i < this->connections.size(); i++) {
			if ((this->wormSockets)[i] > 0) {
				close((this->wormSockets) [i]);
			}
		}

		free(this->wormSockets);
	}

	if (this->fdinfo != 0) {
		free(this->fdinfo);
	}
}

void Connection::createWorm(shared_ptr<Worm> wc)
{
	// TODO: Conectarse al remoto y crear worm
	mutex_lock();

	this->connections.insert(make_pair(wc->ws.id, wc));
	this->numWormSockets = this->connections.size();
	void *ret = realloc(static_cast<void *>(wormSockets), this->numWormSockets * sizeof(int));

	if (ret == 0) {
		mutex_unlock();
		throw std::runtime_error("Error reallocating socket array");
	}

	this->wormSockets = static_cast<int *>(ret);
	this->wormSockets[this->numWormSockets - 1] = 0;


	ret = realloc(static_cast<void *>(fdinfo), this->connections.size() * sizeof(struct pollfd));

	if (ret == 0) {
		mutex_unlock();
		throw std::runtime_error("Error reallocating poll array");
	}

	this->fdinfo = static_cast<struct pollfd *>(ret);
	bzero((char *)(this->fdinfo + (this->numWormSockets - 1)), sizeof(struct pollfd));

	mutex_unlock();
}


void Connection::deleteWorm(const uint16_t id)
{
	cerr << "Deleting worm id = " << id << endl;

	this->connections.erase(id);
}

void Connection::deleteAllWorms()
{
	cerr << "Deleting all worms" << endl;

	while (!this->connections.empty()) {
		try {
			this->deleteWorm(this->connections.rbegin()->first);

		} catch (exception e) {
			cerr << "Error halting worm " << this->connections.rbegin()->second->ws.id << ": " << e.what();
		}
	}

	cerr << "Done" << endl;
}

void Connection::run()
{
	// Deploy worms
	if (this->autoDeployWorms) {
		for (auto connIterator = this->connections.begin(); connIterator != this->connections.end(); connIterator++) {
			deployWorm(*(connIterator->second));
		}
	}

	setupWormThread();

	cerr << "Completed deployment of all worms" << endl;

	for (;;) {

		mutex_lock();
		pollWorms();
		mutex_unlock();

		if (!keepRunning) {
			break;
		}

		usleep(10);
	}

	//throw std::runtime_error("Forcing to delete Einstein"); //TODO: Is necesary?
}

int Connection::setupWorm()
{
	struct timeval ts;
	ts.tv_sec  =   0; //TODO parametrizar esta variable
	ts.tv_usec =   100;

	int currentWormSocket = tcp_accept(this->listeningSocket, &ts);

	if (currentWormSocket == -1) {
		//throw std::runtime_error("Error accepting connection");
		return 1;
	}

	// Get hello message
	size_t hellomsgSize = sizeof(enum ctrlMsgType) + sizeof(uint16_t);
	uint8_t hellomsg[hellomsgSize];

	if (tcp_message_recv(currentWormSocket, hellomsg, hellomsgSize, 1) != (ssize_t)hellomsgSize) {
		throw std::runtime_error("Error receiving message");
	}

	enum ctrlMsgType *msgType = reinterpret_cast<enum ctrlMsgType *>(hellomsg);

	if (*msgType != HELLOEINSTEIN) {
		return 1;
	}

	uint16_t wormId = ntohs(* ((uint16_t *)(hellomsg + sizeof(enum ctrlMsgType))));

	// Send configuration message
	const void *wormSetup = static_cast<const void *>(& (this->connections.at(wormId)->ws));

	*msgType = SETUP;

	if (tcp_message_send(currentWormSocket, msgType, sizeof(enum ctrlMsgType)) != 0) {
		throw std::runtime_error("Error sending SETUP message");
	}

	if (tcp_message_send(currentWormSocket, wormSetup, sizeof(WormSetup)) != 0) {
		throw std::runtime_error("Error sending message");
	}

	const void *connDescription = static_cast<const void *>(this->connections.at(wormId)->ws.connectionDescription);

	if (this->connections.at(wormId)->ws.connectionDescriptionLength > 0) {
		if (tcp_message_send(currentWormSocket, connDescription, this->connections.at(wormId)->ws.connectionDescriptionLength) != 0) {
			throw std::runtime_error("Error sending message");
		}
	}

	mutex_lock();

	connectWorm(wormId, currentWormSocket);

	mutex_unlock();

	cerr << "Completed setup of worm " << wormId << endl;
	return 0;
}

void Connection::connectWorm(const uint16_t id, const int socket)
{
	this->connections.at(id)->socket = socket;

	// Add socket to the list used for polling
	int socketIndex = distance(this->connections.begin() , this->connections.find(id));
	wormSockets[socketIndex] = socket;

	// TODO: Insert socket descriptor in property wormSockets
}

void Connection::deployWorm(Worm &wc)
{
	//Check if alredy deployed
	auto v = deployedWorms.find(wc.host);
	bool copyData = true;;

	if (v == deployedWorms.end()) {
		set<string> tmpset;
		tmpset.insert(wc.programName);
		deployedWorms.insert(pair<string, set<string>>(wc.host, tmpset));

	} else {
		auto p = v->second.find(wc.programName);

		if (p == v->second.end()) {
			v->second.insert(wc.programName);

		} else {
			cerr << "Program \"" << wc.programName << "\" alredy copied to worm " << wc.ws.id << endl;
			copyData = false;
		}
	}

	string executable = "";

	if (copyData) {
		cerr << "Copying program \"" << wc.programName << "\" to worm " << wc.ws.id << endl;
		executable = "rsync -au " + wc.programName + ".tgz '[" + wc.host + "]':~";

		if (system(executable.c_str())) {
			cerr << "error executing comand: \"" << executable << "\"" << endl;
		}
	}

	executable = "ssh -T " + wc.host + " 'tar -xzf " + wc.programName + ".tgz;"
				 "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/" + wc.programName + "/lib;"
				 "export WORM_ID=" + std::to_string(wc.ws.id) + ";"
				 "export EINSTEIN_PORT=" + std::to_string(this->listenPort) + ";"
				 "export EINSTEIN_IP=" + this->listenIpStr + ";"
				 "cd " + wc.programName + ";"
				 "nohup sh run.sh ";

	if (wc.runParams.size() > 0) {
		for (unsigned i = 0; i < wc.runParams.size(); i++) {
			executable += "\"" + wc.runParams[i] + "\" ";
		}
	}

	executable += " > /dev/null 2>&1 &'";

	if (system(executable.c_str())) {
		cerr << "error executing comand: \"" << executable << "\"" << endl;
	}

	wc.deployed = true;
}

void Connection::setupWormThread()
{
	this->setupThread = thread(
	[this] {
		while (keepRunning) {
			// TODO: Relanzar worm si pasa mucho tiempo sin responder
			this->setupWorm();
			usleep(10);
		}
	});
}

void Connection::pollWorms()
{

	int i = 0, j = 0;

	// Add all socket descriptors to a poll array. Try to reconnect if one of the sockets is closed.
	for (i = 0, j = 0; j < this->numWormSockets; ++i, ++j) {
		memset(& (this->fdinfo[i]), 0, sizeof(struct pollfd));

		if (this->wormSockets[j] == -1) { // The connection to the worm was closed
			// Relaunch worm
			auto connIterator = connections.begin();

			for (int k = 0; k < j; ++k) {
				connIterator++;
			}

			this->wormSockets[j] = 0;
			// Try to deploy the worm. The setup thread will eventually finish the connection
			deployWorm(*(connIterator->second));

		} else {
			this->fdinfo[i].fd = this->wormSockets[j];
			this->fdinfo[i].events = POLLIN | POLLHUP | POLLRDNORM | POLLNVAL;
		}

		if (this->wormSockets[j] == 0) { // The socket was not prepared. Skip it
			--i;
		}
	}

	this->numFilledPolls = i;


	if (this->numFilledPolls > 0) {
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
							close(this->wormSockets[i]);
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
								close(this->wormSockets[i]);
								this->wormSockets[i] = -1;
								continue;
							}

							if (tcp_message_send(this->fdinfo[i].fd, wormSetup, sizeof(WormSetup)) != 0) {
								// Closed socket
								close(this->wormSockets[i]);
								this->wormSockets[i] = -1;
								continue;
							}

						} catch (std::out_of_range &e) {
							// Send error
							enum ctrlMsgType errorMsg = CTRL_ERROR;
							cerr << "Worm " << wormId << " does not exist" << endl;

							if (tcp_message_send(this->fdinfo[i].fd, reinterpret_cast<uint8_t *>(&errorMsg), sizeof(enum ctrlMsgType)) != 0) {
								// Closed socket
								close(this->wormSockets[i]);
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
									break;
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

					case CTRL_ERROR:

						// TODO better implementation
						for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
							if (it->second->socket == this->wormSockets[i]) {
								cerr << "ERROR MSG from worm.id = " << it->first << endl;
								break;
							}
						}

						break;

					case CTRL_OK:

						// TODO better implementation
						for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
							if (it->second->socket == this->wormSockets[i]) {
								cerr << "OK MSG from worm.id = " << it->first << endl;
								break;
							}
						}

						break;

					case PING: {
							// TODO better implementation
							for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
								if (it->second->socket == this->wormSockets[i]) {
									cerr << "PING MSG from worm.id = " << it->first << endl;
									break;
								}
							}

							enum ctrlMsgType msg = PONG;
							tcp_message_send(this->wormSockets[i], &msg, sizeof(msg));

							break;
						}

					case PONG:

						// TODO better implementation
						for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
							if (it->second->socket == this->wormSockets[i]) {
								cerr << "PONG MSG from worm.id = " << it->first << endl;
								break;
							}
						}

						break;


					case PRINTMSG: {
							uint32_t msgsize = 0;
							tcp_message_recv(this->fdinfo[i].fd, &msgsize, sizeof(uint32_t), 1);

							if (msgsize) {
								char *tmpstr = (char *)malloc(msgsize + 1);
								tcp_message_recv(this->fdinfo[i].fd, tmpstr, msgsize, 1);
								tmpstr[msgsize] = 0;

								// TODO better implementation
								for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
									if (it->second->socket == this->wormSockets[i]) {
										cerr << "MSG from worm.id " << it->first << ":" << tmpstr << endl;
										break;
									}
								}

								free(tmpstr);

							} else {
								// TODO better implementation
								for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
									if (it->second->socket == this->wormSockets[i]) {
										cerr << "EMPTY MSG from worm.id " << it->first << endl;
										break;
									}
								}
							}

							break;
						}

					case ABORT: {
							uint32_t msgsize = 0;
							tcp_message_recv(this->fdinfo[i].fd, &msgsize, sizeof(uint32_t), 1);

							if (msgsize) {
								char *tmpstr = (char *)malloc(msgsize + 1);
								tcp_message_recv(this->fdinfo[i].fd, tmpstr, msgsize, 1);
								tmpstr[msgsize] = 0;

								// TODO better implementation
								for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
									if (it->second->socket == this->wormSockets[i]) {
										cerr << "ABORT from worm.id " << it->first << ". Cause:" << tmpstr << endl;
										break;
									}
								}

								free(tmpstr);

							} else {
								// TODO better implementation
								for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
									if (it->second->socket == this->wormSockets[i]) {
										cerr << "ABORT from worm.id " << it->first << endl;
										break;
									}
								}
							}

							this->keepRunning = false;
							break;
						}

					default:
						// Send error
						enum ctrlMsgType errorMsg = CTRL_ERROR;

						// TODO better implementation
						for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
							if (it->second->socket == this->wormSockets[i]) {
								cerr << "NON IMPLEMENTED MSG (" << ctrlMsgType2str(ctrlMsg) << ") from worm.id " << it->first << endl;
								break;
							}
						}

						if (tcp_message_send(this->fdinfo[i].fd, reinterpret_cast<uint8_t *>(&errorMsg), sizeof(enum ctrlMsgType)) != 0) {
							// Closed socket
							close(this->wormSockets[i]);
							this->wormSockets[i] = -1;
							continue;
						}
					}

				} else if (this->fdinfo[i].revents & POLLHUP || this->fdinfo[i].revents & POLLRDNORM || this->fdinfo[i].revents & POLLNVAL) {
					close(this->wormSockets[i]);
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

}

void Connection::mutex_lock()
{
	mtx.lock();
}
void Connection::mutex_unlock()
{
	mtx.unlock();
}
