#include <einstein/einsshell.hpp>

using namespace einstein;

EinsShell::EinsShell(shared_ptr<Einstein> eins)
{
	this->eins = eins;
}

EinsShell::~EinsShell()
{

}

int EinsShell::startShell()
{
	int ret = 0;

	sleep(20);

	return ret;
}
