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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int message_count;
	int message_size;
	int rc;
	int i, j;

	if (WH_init()) {
		perror("Error WH_init");
		return -1;  // Error initializing libworm
	}
	ConnectionDataType type = {.type = ARRAY, .ext.arrayType = UINT8};
	WH_setup_types(1, &type);

	if (argc != 3) {
		WH_abortf("usage: local_thr <message-size> <message-count>\n");
		return 1;
	}
	message_size  = atoi(argv[1]);
	message_count = atoi(argv[2]);

	void *buffer = calloc(message_size, 1);
	hptl_t begin, end;
	uint64_t elapsed;

	for (i = 0; i != message_count; i++) {
		MessageInfo mi;
		mi.size = message_size;
		mi.type = &type;

		rc = WH_recv(buffer, &mi);
		if (rc != message_size) {
			WH_abortf("error in WH_recv / Message size mismatch (%d/%d)\n", rc, message_size);
		}
		if (i == 0)
			begin = hptl_get();
	}

	end     = hptl_get();
	elapsed = hptl_ntimestamp(end - begin) / 1000;  // to us

	double throughput = ((double)message_count / (double)elapsed * 1000000);
	double megabits   = ((double)throughput * message_size * 8) / 1000000;

	WH_printf("message size: %d [B]\n", (int)message_size);
	WH_printf("message count: %d\n", (int)message_count);
	WH_printf("mean throughput: %d [msg/s]\n", (int)throughput);
	WH_printf("mean throughput: %.3f [Mb/s]\n", (double)megabits);

	WH_halt();

	return 0;
}