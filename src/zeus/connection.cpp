/* Copyright (c) 2015-2018 Rafael Leira, Paula Roquero, Naudit HPCN
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <zeus/connection.hpp>

using namespace zeus;

bool Connection::keepRunning = true;

Connection::Connection(string listenIp, uint16_t listenPort) : Connection(listenIp, listenPort, true)
{
}

Connection::Connection(const string listenIp, const uint16_t listenPort, bool autoDeployHoles)
{
	// Start socket to receive connections from holes
	this->listenIpStr     = listenIp;
	this->listenIp        = inet_addr(listenIp.c_str());
	this->listenPort      = listenPort;
	this->listeningSocket = unique_ptr<SSocket>(new SSocket());

	if (this->listeningSocket == nullptr) {
		throw std::runtime_error("Error listening to socket");
	}

	this->listeningSocket->listen(listenPort);

	this->holeSockets       = 0;
	this->fdinfo            = 0;
	this->numHoleSockets    = 0;
	this->previousPollIndex = 0;
	this->autoDeployHoles   = autoDeployHoles;

	signal((int)SIGINT, Connection::signal_callback_handler);
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

	this->deleteAllHoles();

	if (this->holeSockets != 0) {
		for (size_t i = 0; i < this->connections.size(); i++) {
			if ((this->holeSockets)[i] > 0) {
				close((this->holeSockets)[i]);
			}
		}

		free(this->holeSockets);
	}

	if (this->fdinfo != 0) {
		free(this->fdinfo);
	}
}

void Connection::createHole(shared_ptr<Hole> wc)
{
	// TODO: Conectarse al remoto y crear hole
	mutex_lock();

	this->connections.insert(make_pair(wc->ws.id, wc));
	this->numHoleSockets = this->connections.size();
	void *ret            = realloc(static_cast<void *>(holeSockets), this->numHoleSockets * sizeof(int));

	if (ret == 0) {
		mutex_unlock();
		throw std::runtime_error("Error reallocating socket array");
	}

	this->holeSockets                           = static_cast<int *>(ret);
	this->holeSockets[this->numHoleSockets - 1] = 0;

	ret = realloc(static_cast<void *>(fdinfo), this->connections.size() * sizeof(struct pollfd));

	if (ret == 0) {
		mutex_unlock();
		throw std::runtime_error("Error reallocating poll array");
	}

	this->fdinfo = static_cast<struct pollfd *>(ret);
	bzero((char *)(this->fdinfo + (this->numHoleSockets - 1)), sizeof(struct pollfd));

	mutex_unlock();
}

void Connection::deleteHole(const uint16_t id)
{
	cerr << "Deleting hole id = " << id << endl;

	this->connections.erase(id);
}

void Connection::deleteAllHoles()
{
	cerr << "Deleting all holes" << endl;

	while (!this->connections.empty()) {
		try {
			this->deleteHole(this->connections.begin()->first);

		} catch (exception &e) {
			cerr << "Error halting hole " << this->connections.rbegin()->second->ws.id << ": " << e.what();
		}
	}

	cerr << "Done" << endl;
}

void Connection::run()
{
	// Deploy holes
	if (this->autoDeployHoles) {
		for (auto connIterator = this->connections.begin(); connIterator != this->connections.end(); connIterator++) {
			deployHole(*(connIterator->second));
		}
	}

	setupHoleThread();

	cerr << "Completed deployment of all holes" << endl;

	for (;;) {
		mutex_lock();
		pollHoles();
		mutex_unlock();

		if (!keepRunning) {
			break;
		}

		usleep(10);
	}

	// throw std::runtime_error("Forcing to delete Zeus"); //TODO: Is necesary?
}

int Connection::setupHole()
{
	struct timeval ts;
	ts.tv_sec  = 0;  // TODO parametrizar esta variable
	ts.tv_usec = 100;

	unique_ptr<SSocket> currentHoleSocket = unique_ptr<SSocket>(this->listeningSocket->accept(&ts));

	if (currentHoleSocket == nullptr) {
		// throw std::runtime_error("Error accepting connection");
		return 1;
	}

	// Get hello message
	size_t hellomsgSize = sizeof(enum ctrlMsgType) + sizeof(uint16_t);
	uint8_t hellomsg[hellomsgSize];

	if (currentHoleSocket->recv(hellomsg, hellomsgSize, 1) != (ssize_t)hellomsgSize) {
		throw std::runtime_error("Error receiving message");
	}

	enum ctrlMsgType *msgType = reinterpret_cast<enum ctrlMsgType *>(hellomsg);

	if (*msgType != HELLOZEUS) {
		return 1;
	}

	uint16_t holeId = ntohs(*((uint16_t *)(hellomsg + sizeof(enum ctrlMsgType))));

	// Send configuration message
	const void *holeSetup = static_cast<const void *>(&(this->connections.at(holeId)->ws));

	*msgType = SETUP;

	if (currentHoleSocket->send(msgType, sizeof(enum ctrlMsgType)) != 0) {
		throw std::runtime_error("Error sending SETUP message");
	}

	if (currentHoleSocket->send(holeSetup, sizeof(HoleSetup)) != 0) {
		throw std::runtime_error("Error sending message");
	}

	const void *connDescription = static_cast<const void *>(this->connections.at(holeId)->ws.connectionDescription);

	if (this->connections.at(holeId)->ws.connectionDescriptionLength > 0) {
		if (currentHoleSocket->send(connDescription, this->connections.at(holeId)->ws.connectionDescriptionLength) != 0) {
			throw std::runtime_error("Error sending message");
		}
	}

	mutex_lock();

	connectHole(holeId, move(currentHoleSocket));

	mutex_unlock();

	cerr << "Completed setup of hole " << holeId << endl;
	return 0;
}

void Connection::connectHole(const uint16_t id, unique_ptr<SSocket> socket)
{
	int fd                           = socket->getFd();
	this->connections.at(id)->socket = move(socket);
	this->connections.at(id)->setTimeoutResponse(2);

	// Add socket to the list used for polling
	int socketIndex          = distance(this->connections.begin(), this->connections.find(id));
	holeSockets[socketIndex] = fd;

	// TODO: Insert socket descriptor in property holeSockets
}

void Connection::deployHole(Hole &wc)
{
	// Check if alredy deployed
	auto v        = deployedHoles.find(wc.host);
	bool copyData = true;

	if (!wc.autoDeploy)  // If not autodeploy skip this
		return;

	if (wc.halting)  // if halting, do not try to re-deploy
		return;

	if (v == deployedHoles.end()) {
		set<string> tmpset;
		tmpset.insert(wc.programName);
		deployedHoles.insert(pair<string, set<string>>(wc.host, tmpset));

	} else {
		auto p = v->second.find(wc.programName);

		if (p == v->second.end()) {
			v->second.insert(wc.programName);

		} else {
			cerr << "Program \"" << wc.programName << "\" alredy copied to hole " << wc.ws.id << endl;
			copyData = false;
		}
	}

	string executable = "";

	if (copyData) {
		cerr << "Copying program \"" << wc.programName << "\" to hole " << wc.ws.id << endl;
		executable = "rsync -au " + wc.programName + ".tgz '[" + wc.host + "]':~";

		if (system(executable.c_str())) {
			cerr << "error executing comand: \"" << executable << "\"" << endl;
		}
	}

	executable = "ssh -T " + wc.host + " 'tar -xzf " + wc.programName +
	             ".tgz;"
	             "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/" +
	             wc.programName +
	             "/lib;"
	             "export HOLE_ID=" +
	             std::to_string(wc.ws.id) +
	             ";"
	             "export ZEUS_PORT=" +
	             std::to_string(this->listenPort) +
	             ";"
	             "export ZEUS_IP=" +
	             this->listenIpStr +
	             ";"
	             "cd " +
	             wc.programName +
	             ";"
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

void Connection::setupHoleThread()
{
	this->setupThread = thread([this] {
		while (keepRunning) {
			// TODO: Relanzar hole si pasa mucho tiempo sin responder
			this->setupHole();
			usleep(10);
		}
	});
}

void Connection::pollHoles()
{
	int i = 0, j = 0;

	// Add all socket descriptors to a poll array. Try to reconnect if one of the sockets is closed.
	for (i = 0, j = 0; j < this->numHoleSockets; ++i, ++j) {
		memset(&(this->fdinfo[i]), 0, sizeof(struct pollfd));

		if (this->holeSockets[j] == -1) {  // The connection to the hole was closed
			// Relaunch hole
			auto connIterator = connections.begin();

			for (int k = 0; k < j; ++k) {
				connIterator++;
			}

			this->holeSockets[j] = 0;
			// Try to deploy the hole. The setup thread will eventually finish the connection
			deployHole(*(connIterator->second));

		} else {
			this->fdinfo[i].fd     = this->holeSockets[j];
			this->fdinfo[i].events = POLLIN | POLLHUP | POLLRDNORM | POLLNVAL;
		}

		if (this->holeSockets[j] == 0) {  // The socket was not prepared. Skip it
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

				if (this->holeSockets[i] == -1) {
					// TODO: Try to reconnect. If it doesn't work launch hole again
				}

				if (this->fdinfo[i].revents & POLLIN) {
					enum ctrlMsgType ctrlMsg;

					if (tcp_message_recv(
					        this->fdinfo[i].fd, reinterpret_cast<uint8_t *>(&ctrlMsg), sizeof(enum ctrlMsgType), 1) !=
					    sizeof(enum ctrlMsgType)) {
						// Closed socket
						this->holeSockets[i] = -1;
						continue;
					}

					// Check message and do corresponding action
					switch (ctrlMsg) {
						case QUERYID:
							// Get hole id
							uint16_t holeId;

							if (tcp_message_recv(this->fdinfo[i].fd, static_cast<void *>(&holeId), sizeof(uint16_t), 1) !=
							    sizeof(uint16_t)) {
								// Closed socket
								close(this->holeSockets[i]);
								this->holeSockets[i] = -1;
								continue;
							}

							// Send hole configuration message
							try {
								const void *holeSetup  = static_cast<const void *>(&(this->connections.at(holeId)->ws));
								enum ctrlMsgType okMsg = CTRL_OK;

								cerr << "Received request for information of hole " << holeId << endl;

								if (tcp_message_send(this->fdinfo[i].fd,
								                     reinterpret_cast<uint8_t *>(&okMsg),
								                     sizeof(enum ctrlMsgType)) != 0) {
									// Closed socket
									close(this->holeSockets[i]);
									this->holeSockets[i] = -1;
									continue;
								}

								if (tcp_message_send(this->fdinfo[i].fd, holeSetup, sizeof(HoleSetup)) != 0) {
									// Closed socket
									close(this->holeSockets[i]);
									this->holeSockets[i] = -1;
									continue;
								}

							} catch (std::out_of_range &e) {
								// Send error
								enum ctrlMsgType errorMsg = CTRL_ERROR;
								cerr << "Hole " << holeId << " does not exist" << endl;

								if (tcp_message_send(this->fdinfo[i].fd,
								                     reinterpret_cast<uint8_t *>(&errorMsg),
								                     sizeof(enum ctrlMsgType)) != 0) {
									// Closed socket
									close(this->holeSockets[i]);
									this->holeSockets[i] = -1;
									continue;
								}
							}

							break;

						case HALT: {  // TODO better implementation
							for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
								if (it->second->socket == this->holeSockets[i]) {
									it->second->halting = true;
									cerr << "Hole's " << it->first << " task ended." << endl;
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
								cerr << "Closing Zeus: All the tasks were completed" << endl;
								this->deleteAllHoles();
								exit(0);  // TODO cambiar por un cierre mas ordenado, como por ejemplo, modificando la
								          // variable de salida utilizada para el cntl+c
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
								if (it->second->socket == this->holeSockets[i]) {
									cerr << "ERROR MSG from hole.id = " << it->first << endl;
									break;
								}
							}

							break;

						case CTRL_OK:

							// TODO better implementation
							for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
								if (it->second->socket == this->holeSockets[i]) {
									cerr << "OK MSG from hole.id = " << it->first << endl;
									break;
								}
							}

							break;

						case PING: {
							// TODO better implementation
							for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
								if (it->second->socket == this->holeSockets[i]) {
									cerr << "PING MSG from hole.id = " << it->first << endl;
									break;
								}
							}

							enum ctrlMsgType msg = PONG;
							tcp_message_send(this->holeSockets[i], &msg, sizeof(msg));

							break;
						}

						case PONG:

							// TODO better implementation
							for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
								if (it->second->socket == this->holeSockets[i]) {
									cerr << "PONG MSG from hole.id = " << it->first << endl;
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
									if (it->second->socket == this->holeSockets[i]) {
										cerr << "MSG from hole.id " << it->first << ": " << tmpstr << endl;
										break;
									}
								}

								free(tmpstr);

							} else {
								// TODO better implementation
								for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
									if (it->second->socket == this->holeSockets[i]) {
										cerr << "EMPTY MSG from hole.id " << it->first << endl;
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
									if (it->second->socket == this->holeSockets[i]) {
										cerr << "ABORT from hole.id " << it->first << ". Cause:" << tmpstr << endl;
										break;
									}
								}

								free(tmpstr);

							} else {
								// TODO better implementation
								for (auto it = this->connections.begin(); it != this->connections.end(); ++it) {
									if (it->second->socket == this->holeSockets[i]) {
										cerr << "ABORT from hole.id " << it->first << endl;
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
								if (it->second->socket == this->holeSockets[i]) {
									cerr << "NON IMPLEMENTED MSG (" << ctrlMsgType2str(ctrlMsg) << ") from hole.id "
									     << it->first << endl;
									break;
								}
							}

							if (tcp_message_send(this->fdinfo[i].fd,
							                     reinterpret_cast<uint8_t *>(&errorMsg),
							                     sizeof(enum ctrlMsgType)) != 0) {
								// Closed socket
								close(this->holeSockets[i]);
								this->holeSockets[i] = -1;
								continue;
							}
					}

				} else if (this->fdinfo[i].revents & POLLHUP || this->fdinfo[i].revents & POLLRDNORM ||
				           this->fdinfo[i].revents & POLLNVAL) {
					close(this->holeSockets[i]);
					this->holeSockets[i] = -1;
				}
			}

		} else {
			for (int i = 0; i < this->numFilledPolls; ++i) {
				if (this->holeSockets[i] == -1) {
					// TODO: Try to reconnect. If it doesn't work launch hole again
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
