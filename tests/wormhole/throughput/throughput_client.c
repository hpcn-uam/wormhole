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
#include <messages.h>
#include <worm.h>
#include <worm_low.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ConnectionDataType type = {.type = ARRAY, .ext.arrayType = UINT8};

int main(int argc, char *argv[])
{
	int message_count;
	int message_size;
	int rc;
	int i, j;
	void *s;

	if (WH_init()) {
		perror("Error WH_init");
		return -1;  // Error initializing libworm
	}
	WH_setup_types(1, &type);

	if (argc != 3) {
		WH_abortf("Params: %s <message-size> <message-count>\n", argv[0]);
		return 1;
	}
	message_size  = atoi(argv[1]);
	message_count = atoi(argv[2]);

	void *buffer = calloc(message_size, 1);

	for (i = 0; i != message_count; i++) {
		MessageInfo mi;
		mi.size = message_size;
		mi.type = &type;

		rc = WH_send(buffer, &mi);
		if (rc) {
			WH_abort("error in WH_send\n");
		}
	}

	WH_halt();
	return 0;
}