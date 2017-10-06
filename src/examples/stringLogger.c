
#define _GNU_SOURCE
#include <common.h>
#include <worm.h>
#include <worm_private.h>

#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	ConnectionDataType type = {.type = ARRAY, .ext.arrayType = UCHAR};

	int st = WH_init();
	assert(st == 0);

	uint64_t buffSize    = 1024 * 1024 * 1024UL;
	uint32_t rotationNum = 0;
	uint32_t rotationMax = 1024;
	uint32_t linelen     = 512;
	char* outputPath     = "/tmp/stringlogger";
	char* outputFileFormat;
	char* outputFile;

	char c;
	while ((c = getopt(argc, argv, "p:s:r:h")) != -1) {
		switch (c) {
			case 'p': {  // output path
				outputPath = strdup(optarg);
				if (!outputPath)
					WH_abortf("Strdup failed copying string '%s'.\n", optarg);

				break;
			}
			case 's': {  // buffer size
				sscanf(optarg, PRIu64, &buffSize);

				if (buffSize < 1024) {
					return WH_abort("Error! buffer size is too small\n");
				}
				break;
			}
			case 'h': {
				return WH_abortf("Use: ./%s [-s fileSize (default = " PRId64
				                 ")] [-p outputPath (default = %s)] [-r numberOfPersistedLogs (default = %s)]\n",
				                 argv[0],
				                 buffSize,
				                 outputPath,
				                 rotationMax);
			}
			case '?': {
				if (optopt == 's' || optopt == 't') {
					WH_abortf("Option -%c requires an argument.\n", optopt);

				} else {
					WH_abortf("Unknown option `-%c'.\n", optopt);
				}
			}

			default: {
				return WH_abort(NULL);
			}
		}
	}

	{
		// Create outputPath if not exists
		char* tmp = WH_sprintf("mkdir -p %s", outputPath);

		if (system(tmp))
			WH_abortf("Cant create directory '%s'.\n", outputPath);

		free(tmp);
	}

	WH_setup_types(1, &type);
	MessageInfo mi;

	mi.size = linelen;
	mi.type = &type;

	uint8_t* buffer   = calloc(buffSize, 1);
	uint64_t buffleft = buffSize;
	uint32_t recvret;

	outputFileFormat = WH_sprintf("%s/%%10d.log", outputPath);
	outputFile       = WH_sprintf(outputFileFormat, rotationNum);  // first file

	for (;;) {
		mi.size = buffleft;
		recvret = WH_recv((void*)buffer, &mi);
		if (recvret == 0) {
			// error ?¿
		} else if (recvret < 0) {
			// Flush file
			buffer -= buffleft;  // set buffer to begining

			// Write the file
			int fd = open(outputFile, O_CREAT | O_WRONLY | O_DIRECT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
			if (write(fd, buffer, buffSize))
				WH_printf("Cant write %d Bytes into file '%s' (errno=%d)\n", buffSize, outputFile, errno);
			if (ftruncate(fd, buffSize - (buffleft - recvret)))
				WH_printf("Cant truncate file '%s' (errno=%d)\n", outputFile, buffSize - (buffleft - recvret), errno);
			close(fd);

			// prepare data for next log
			buffleft = buffSize;  // set buffleft to "max" value
			if (++rotationNum >= rotationMax)
				rotationNum = 0;
			sprintf(outputFile, outputFileFormat, rotationNum);  // first file
		}
		buffer += recvret;
		buffleft -= recvret;
	}

	return WH_halt();
}