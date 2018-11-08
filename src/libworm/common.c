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

#include <common.h>

char *ctrlMsgType2str(enum ctrlMsgType msg)
{
	switch (msg) {
		case HELLOZEUS:
			return "HELLOZEUS";

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