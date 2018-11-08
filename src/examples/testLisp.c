/* Copyright (c) 2015-2018 Rafael Leira
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
#include <worm.h>
#include <worm_private.h>

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include <malloc.h>

extern Worm2EinsConn WH_einsConn;
extern uint16_t WH_myId;
extern HoleSetup WH_mySetup;
extern DestinationHoles WH_myDstWorms;

#define TESTLIST "(Cat (1.(DUP 1 2 3)) (2.(RR 1 2 3)))"
#define TESTDATA "1234567890"

int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);

	int st = WH_init();
	assert(st == 0);

	assert(!memcmp(WH_mySetup.connectionDescription, TESTLIST, strlen(TESTLIST)));
	fprintf(stderr, "Éxito setup\n");

	MessageInfo mi;
	ConnectionDataType type;
	type.type          = ARRAY;
	type.ext.arrayType = UINT8;

	mi.size     = strlen(TESTDATA) + 1;
	mi.type     = &type;
	mi.category = 1;

	st = WH_send(TESTDATA, &mi);
	assert(st == 0);
	fprintf(stderr, "Mensajes enrutados!\n");

	// st = WH_flushIO();
	// assert(st == 0);

	// sleep(5);
	// TODO que sucede con un doble flush??
	// st = WH_flushIO();
	// assert(st == 0);

	char *data = (char *)malloc(1000);

	WH_recv(data, &mi);
	fprintf(stderr, "RECVMSG: %s\n", data);
	WH_recv(data, &mi);
	fprintf(stderr, "RECVMSG: %s\n", data);
	WH_recv(data, &mi);
	fprintf(stderr, "RECVMSG: %s\n", data);

	fprintf(stderr, "Mensajes recibidos!\n");

	return WH_halt();
}
