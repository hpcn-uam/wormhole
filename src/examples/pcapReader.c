#include <worm.h>
#include <worm_private.h>
#include <common.h>

#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <malloc.h>

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

	if (argc == 1) {
		fprintf(stderr, "Not file provided\n");
		return WH_halt();
	}

	FILE *pcap = fopen(argv[1], "r");

	if (pcap == NULL) {
		fprintf(stderr, "Not file provided 2\n");
		return WH_halt();
	}

	//read header
	if (fseek(pcap, sizeof(pcap_hdr_tJZ), SEEK_SET)) {
		fprintf(stderr, "fseek error\n");
		return WH_halt();
	}

	uint8_t *buffer = calloc(9200, 1);
	pcaprec_hdr_tJZ *header = (pcaprec_hdr_tJZ *) buffer;
	uint8_t *data = buffer + sizeof(pcaprec_hdr_tJZ);

	int flag = 1;

	MessageInfo mi;
	ConnectionDataType type;
	type.type = ARRAY;
	type.ext.arrayType = UINT8;
	mi.type = &type;
	mi.category = 1;

	while (flag) {
		if (fread(header, sizeof(pcaprec_hdr_tJZ), 1, pcap) <= 0) {
			fprintf(stderr, "pcap ended, finishing...\n");
			break;
		}

		if (fread(data, header->incl_len, 1, pcap) <= 0) {
			fprintf(stderr, "[ERROR]: half packet readed!!!\n");
			break;
		}

		mi.size = header->incl_len;

		if (WH_send(buffer, &mi)) {
			fprintf(stderr, "wh_send error\n");
			flag = 0;
		}

	}

	return WH_halt();
}