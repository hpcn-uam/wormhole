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

#include <zeus/zeus.hpp>
#include <zeus/zeusshell.hpp>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <cassert>
#include <cstring>

using namespace zeus;

int main(int argc, char **argv)
{
	shared_ptr<Zeus> zeus;

	hptl_init(NULL);
	netlib_init(1, argv);

	if (argc == 3) {
		try {
			zeus = make_shared<Zeus>(argv[1], argv[2], 5000, true);

		} catch (exception &e) {
			std::cerr << "Exception: " << e.what() << '\n';
			return 1;
		}

	} else {
		std::cerr << "No pararms provided" << endl;
		std::cerr << "Try: " << argv[0] << " <CONFIG FILE> <IP>" << endl;
		return 1;
	}

	unique_ptr<ZeusShell> cmd(new ZeusShell(zeus));

	std::cerr << "Launching Zeus, please wait..." << endl;
	zeus->openThreadedHoles();
	cmd->waitForZeus();

	int ret = cmd->startShell();

	if (ret)
		std::cerr << "Some error happened...!" << endl << "Closing Zeus & Halting Everything" << endl;

	return ret;
}
