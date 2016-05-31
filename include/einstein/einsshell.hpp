#ifndef __EINSTEIN_SHELL_H__
#define __EINSTEIN_SHELL_H__

#include <common.h>
#include <einstein/einstein.hpp>

#include <poll.h>
#include <signal.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <stdexcept>

using namespace std;

namespace einstein
{
class EinsShell
{
	friend class Einstein;
	shared_ptr<Einstein> eins;

 public:
	EinsShell(shared_ptr<Einstein> eins);
	~EinsShell();

	/*
	 * Starts the Einstein's Shell
	 * @return 0 if normal exit. Other or exception, if some error.
	 */
	int startShell();

 private:

};
}

#endif
