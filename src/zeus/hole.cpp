#include <zeus/hole.hpp>

using namespace zeus;

Hole::Hole(uint16_t id, uint16_t listenPort, int16_t core, string connectionDescription, string host, string programName)
    : Hole(id, listenPort, core, connectionDescription, host, programName, vector<string>())
{
}

Hole::Hole(uint16_t id,
           uint16_t listenPort,
           int16_t core,
           string connectionDescription,
           string host,
           string programName,
           vector<string> runParams)
{
	connectionDescription = Hole::expandCDescription(connectionDescription);
	memset(&this->ws, 0, sizeof(this->ws));

	this->ws.id                          = id;
	this->ws.listenPort                  = listenPort;
	this->ws.connectionDescriptionLength = connectionDescription.size();
	this->ws.connectionDescription       = (uint8_t *)strdup(connectionDescription.c_str());
	this->ws.core                        = core;

	// flags
	this->ws.isSSLNode = 0;  // false
	this->ws.isIPv6    = 0;
	this->ws.zeusSSL   = 0;

	// version
	this->ws.zeusVersion = ZEUSVERSION;
	this->ws.holeVersion = WORMVERSION;

	this->host        = host;
	this->programName = programName;
	this->halting     = false;
	this->autoDeploy  = true;
	this->deployed    = false;

	this->runParams = runParams;

	this->setIP(this->host);
	this->setTimeoutResponse(2);  // default timeout set to 2
}

Hole::~Hole() noexcept(false)
{
	free(this->ws.connectionDescription);

	if (this->deployed) {
		if (this->socket != nullptr) {
			ctrlMsgType msg = HALT;
			cerr << "Hole id = " << this->ws.id << " HALTED" << endl;

			if (this->socket->send(&msg, sizeof(msg)) != 0) {
				throw std::runtime_error("Error sending HALT");
			}
		} else {
			cerr << "Hole with id = " << this->ws.id << " deployed but never connected to Zeus..." << endl;
		}

	} else {
		cerr << "Hole with id = " << this->ws.id << " has not alredy been deployed (UNHALTED)..." << endl;
	}
}

void Hole::setIP(string iphostname)
{
	// check if ipaddr or name
	int result4 = 0, result6 = 0;

	result4 = inet_pton(AF_INET, iphostname.c_str(), &(this->ws.IP));

	if (result4 == -1 || result4 == 0) {  // try IPv6
		result6 = inet_pton(AF_INET6, iphostname.c_str(), &(this->ws.IP));
	}

	if ((result4 == -1 || result4 == 0) && (result6 == -1 || result6 == 0)) {
		struct addrinfo *result;
		struct addrinfo *res;
		int error;

		error = getaddrinfo(iphostname.c_str(), NULL, NULL, &result);

		if (error) {  // error
			throw std::runtime_error("Host '" + string(host) + "' does not exists");
		}

		int flag = 0;

		for (res = result; res != NULL; res = res->ai_next) {
			if (res->ai_addr != NULL) {
				flag = 1;
				memcpy(&(this->ws.IP), res->ai_addr, res->ai_addrlen);

				if (res->ai_family == AF_INET6) {
					this->ws.isIPv6 = 1;
				}

				break;
			}
		}

		if (result) {
			freeaddrinfo(result);
		}

		if (!flag) {
			throw std::runtime_error("Host '" + string(host) + "' does not have a valid IP address");
		}

	} else if (result4 == -1 || result4 == 0) {
		this->ws.isIPv6 = 1;
	}
}

bool Hole::setTimeoutResponse(time_t seconds)
{
	if (this->socket == nullptr) {
		return false;
	}

	try {
		struct timeval ts = {.tv_sec = seconds, .tv_usec = 0};  // timeout at 2 seconds
		this->socket->setSocketTimeout(&ts);
		return true;

	} catch (exception e) {
		cerr << "Warning, timeout failed" << endl;
		return false;
	}
}

ostream &zeus::operator<<(ostream &os, Hole const &obj)
{
	os << "ID: " << obj.ws.id << " ADDR: " << obj.host << ":" << obj.ws.listenPort << (obj.ws.isSSLNode ? " [SSL]" : "")
	   << " | " << obj.programName << " " << endl
	   << "\t Route: " << obj.ws.connectionDescription;

	if (obj.runParams.size() > 0) {
		os << endl;
		os << "\t Params:";

		for (auto param : obj.runParams) {
			os << " " << param;
		}
	}

	return os;
}

string Hole::expandCDescription(string cd)
{
	string ret = cd;

	// clean the string
	char removableChars[] = "\t\r\n\"'";

	for (unsigned int i = 0; i < strlen(removableChars); i++) {
		ret.erase(remove(ret.begin(), ret.end(), removableChars[i]), ret.end());
	}

	if (ret.length() == 0) {  // TODO do more checks
		ret = "IGNORE";
	}

	return ret;
}

int64_t Hole::kill()
{
	if (this->socket == nullptr) {
		return 0;
	}

	this->halting = true;

	ctrlMsgType msg = HALT;
	this->socket->send(&msg, sizeof(msg));
	this->socket = nullptr;

	return 0;
}

int64_t Hole::ping()
{
	if (this->socket == nullptr) {
		return 0;
	}

	hptl_t begin = hptl_get();

	ctrlMsgType msg = PING;

	this->setTimeoutResponse(2);
	this->socket->send(&msg, sizeof(msg));
	msg = TIMEOUT;

	errno = 0;
	this->socket->recv(&msg, sizeof(msg), 0);

	hptl_t end = hptl_get();

	if (msg == TIMEOUT) {
		return -(hptl_ntimestamp(end - begin) / 1000);
	}

	if (msg != PONG) {
		cerr << "[Response: " << ctrlMsgType2str(msg) << "]";
	}

	return hptl_ntimestamp(end - begin) / 1000;
}

uint64_t Hole::chroute(string newRoute)
{
	if (this->socket == nullptr) {
		return 1;
	}

	newRoute = Hole::expandCDescription(newRoute);

	ctrlMsgType msg = CHANGEROUTE;
	uint32_t length = newRoute.length() + 1;
	this->socket->send(&msg, sizeof(msg));
	this->socket->send(&length, sizeof(length));
	this->socket->send(newRoute.c_str(), length);

	this->ws.connectionDescriptionLength = length;
	this->ws.connectionDescription       = (uint8_t *)realloc(this->ws.connectionDescription, length);
	memcpy(this->ws.connectionDescription, newRoute.c_str(), length);

	return 0;
}

#ifdef WH_STATISTICS
vector<ConnectionStatistics> Hole::getStatistics(bool inout)
{
	ctrlMsgType msg = QUERYID;
	vector<ConnectionStatistics> ret;

	enum queryType qtype;
	enum queryType qtype_rep;
	if (inout) {
		// Input statistics
		qtype = qSTATISTICS_IN;
	} else {
		// Output statistics
		qtype = qSTATISTICS_OUT;
	}
	this->socket->send(&msg, sizeof(msg));
	this->socket->send(&qtype, sizeof(qtype));

	this->setTimeoutResponse(5);

	// get the response
	this->socket->recv(&msg, sizeof(msg), 0);
	this->socket->recv(&qtype_rep, sizeof(qtype_rep), 0);
	if (msg == RESPONSEID && qtype == qtype_rep) {
		size_t responseNumber;

		if (this->socket->recv(&responseNumber, sizeof(responseNumber), 0) == sizeof(responseNumber)) {
			for (int i = 0; i < responseNumber; i++) {
				ConnectionStatistics tmp;
				this->socket->recv(&tmp, sizeof(tmp), 0);
				ret.push_back(tmp);
			}
		}
	}
	return ret;
}
#endif