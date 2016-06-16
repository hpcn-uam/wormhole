#include <worm.h>
#include <worm_private.h>
#include <common.h>

#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <malloc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/* Global header */
typedef struct pcap_hdr_s {
	uint32_t magic_number;   /* magic number */
	uint16_t version_major;  /* major version number */
	uint16_t version_minor;  /* minor version number */
	uint32_t thiszone;       /* GMT to local correction */
	uint32_t sigfigs;        /* accuracy of timestamps */
	uint32_t snaplen;        /* max length of captured packets, in octets */
	uint32_t network;        /* data link type */
} pcap_hdr_tJZ;

/* Packet header */
typedef struct pcaprec_hdr_s {
	/* timestamp seconds */
	uint32_t ts_sec;
	/* timestamp microseconds */
	uint32_t ts_usec;
	/* number of octets of packet saved in file */
	uint32_t incl_len;
	/* actual length of packet */
	uint32_t orig_len;
} pcaprec_hdr_tJZ;

int main(int argc, char **argv)
{
	int st = WH_init();
	assert(st == 0);

	int c;
	char *fname = NULL;
	int loop = 1;

	uint8_t *file_cur = NULL;
	uint8_t *file_end = NULL;
	uint8_t *file_start = NULL;

	ConnectionDataType type;
	type.type = ARRAY;
	type.ext.arrayType = UINT8;

	MessageInfo mi;
	mi.type = &type;

	while ((c = getopt(argc, argv, "st:h")) != -1) {
		switch (c) {
		case 'f': //type
			fname = strdup(optarg);
			break;

		case 'l': //size
			loop = atoi(optarg);

			if (loop < 0) {
				return WH_abort("Error! loop is too small\n");
			}

			break;

		case 'h': {
				char tmpmsg[4096];
				sprintf(tmpmsg, "Use: ./%s -f <file name> [-l <times to loop the file)>]\n", argv[0]);
				return WH_abort(tmpmsg);
			}

		case '?': {
				char tmpmsg[4096];

				if (optopt == 's' || optopt == 't') {
					sprintf(tmpmsg, "Option -%c requires an argument.\n", optopt);

				} else {
					sprintf(tmpmsg, "Unknown option `-%c'.\n", optopt);
				}

				return WH_abort(tmpmsg);
			}

		default:
			return WH_abort(NULL);
		}
	}

	if (!fname) {
		return WH_abort("Not file provided!");
	}

	int fd = open(argv[1], O_RDONLY);

	if (fd == -1) {
		return WH_abort("File cant be opened");
	}

	struct stat sb;

	if (fstat(fd, &sb) == -1) {
		return WH_abort("fstat failed");
	}

	file_start = mmap(NULL, sb.st_size, PROT_READ,
					  MAP_PRIVATE | MAP_HUGETLB | MAP_POPULATE, fd, 0);

	if (file_start == MAP_FAILED) {
		return WH_abort("mmap failed");
	}

	file_end = file_start + sb.st_size;
	file_start += sizeof(pcap_hdr_tJZ);

	pcaprec_hdr_tJZ *header;
	uint8_t *data;
	int flag = 1;

	while (loop--) {
		file_cur = file_start;

		while (flag) {
			header = (pcaprec_hdr_tJZ *)file_cur;
			data = file_cur + sizeof(pcaprec_hdr_tJZ);

			mi.hash = data[14 + 19];
			mi.size = header->incl_len;

			if (WH_send(file_cur, &mi)) {
				fprintf(stderr, "wh_send error\n");
				flag = 0;
			}

			file_cur += header->incl_len;

			if (file_cur >= file_end) { //file ended
				break;
			}
		}
	}

	return WH_halt();
}