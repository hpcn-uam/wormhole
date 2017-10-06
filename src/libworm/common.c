#include <common.h>

char *ctrlMsgType2str(enum ctrlMsgType msg)
{
	switch (msg) {
		case HELLOEINSTEIN:
			return "HELLOEINSTEIN";

		case STARTSSL:
			return "STARTSSL";

		case SETUP:
			return "SETUP";

		case QUERYID:
			return "QUERYID";

		case RESPONSEID:
			return "RESPONSEID";

		case PING:
			return "PING";

		case PONG:
			return "PONG";

		case CHANGEROUTE:
			return "CHANGEROUTE";

		case DOWNLINK:
			return "DOWNLINK";

		case OVERLOAD:
			return "OVERLOAD";

		case UNDERLOAD:
			return "UNDERLOAD";

		case CTRL_OK:
			return "CTRL_OK";

		case CTRL_ERROR:
			return "CTRL_ERROR";

		case HALT:
			return "HALT";

		case ABORT:
			return "ABORT";

		case PRINTMSG:
			return "PRINTMSG";

		case TIMEOUT:
			return "TIMEOUT";

		default:
			return "UNKNOWN MSG";
	}
}