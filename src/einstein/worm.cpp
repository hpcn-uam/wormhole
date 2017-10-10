#include <einstein/worm.hpp>

using namespace einstein;

Worm::Worm(uint16_t id, uint16_t listenPort, int16_t core, string connectionDescription, string host, string programName)
    : Worm(id, listenPort, core, connectionDescription, host, programName, vector<string>())
{
}

Worm::Worm(uint16_t id,
           uint16_t listenPort,
           int16_t core,
           string connectionDescription,
           string host,
           string programName,
           vector<string> runParams)
{
	connectionDescription = Worm::expandCDescription(connectionDescription);
	memset(&this->ws, 0, sizeof(this->ws));

	this->ws.id                          = id;
	this->ws.listenPort                  = listenPort;
	this->ws.connectionDescriptionLength = connectionDescription.size();
	this->ws.connectionDescription       = (uint8_t *)strdup(connectionDescription.c_str());
	this->ws.core                        = core;

	// flags
	this->ws.isSSLNode   = 0;  // false
	this->ws.isIPv6      = 0;
	this->ws.einsteinSSL = 0;

	// version
	this->ws.einsteinVersion = EINSTEINVERSION;
	this->ws.wormVersion     = WORMVERSION;

	this->host        = host;
	this->programName = programName;
	this->halting     = false;
	this->deployed    = false;

	this->runParams = runParams;

	this->setIP(this->host);
}

Worm::~Worm()
{
	free(this->ws.connectionDescription);

	if (this->deployed) {
		ctrlMsgType msg = HALT;
		cerr << "Worm id = " << this->ws.id << "HALTED" << endl;

		if (this->socket->send(&msg, sizeof(msg)) != 0) {
			throw std::runtime_error("Error sending HALT");
		}

	} else {
		cerr << "Worm with id = " << this->ws.id << " has not alredy been deployed (UNHALTED)..." << endl;
	}
}

void Worm::setIP(string iphostname)
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

ostream &einstein::operator<<(ostream &os, Worm const &obj)
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

string Worm::expandCDescription(string cd)
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

int64_t Worm::ping()
{
	if (this->socket == nullptr) {
		return 0;
	}

	hptl_t begin = hptl_get();

	ctrlMsgType msg = PING;

	try {
		struct timeval ts = {.tv_sec = 2, .tv_usec = 0};  // timeout at 2 seconds
		this->socket->setSocketTimeout(&ts);

	} catch (exception e) {
		cerr << "Warning, timeout failed, the ping will not stop until pong!" << endl;
	}

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

uint64_t Worm::chroute(string newRoute)
{
	if (this->socket == nullptr) {
		return 1;
	}

	newRoute = Worm::expandCDescription(newRoute);

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
