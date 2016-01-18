#ifndef __WORM_H__
#define __WORM_H__

#include "common.h"

enum DataType {CUSTOM, COMPOUND, ARRAY, INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64, STRING};

typedef struct {
	enum DataType type;
	union ext {
		enum DataType arrayType;
		/*TODO:		struct
				{
					uint32_t size;
					struct ConnectionDataType *elementTypes;
				} compoundType;*/
	}; //extended
} ConnectionDataType;

typedef struct {
	uint16_t category;
	uint16_t priority;
	uint16_t dport;
	uint16_t did;
	uint32_t dip; //TODO fix para ipv6
	int socket;
	ConnectionDataType type;
} Connection;

typedef struct {
	uint32_t size;
	uint32_t hash;
	uint16_t category;
	uint16_t priority;
	ConnectionDataType *type;
} MessageInfo;

/* Name WH_init
 * Starts the WormHole Library
 * Return 0 if OK, something else if error.
 */
uint8_t WH_init (void);

/* Name WH_halt
 * Stops and free the WormHole Library
 * Return 0 if OK, something else if error.
 */
uint8_t WH_halt (void);

/* Name WH_recv
 * TODO
 * Params:
 * Return the number of bytes readed, 0 if ERROR or none.
 */
uint32_t WH_recv (void *data, MessageInfo *mi);

/* Name WH_send
 * TODO
 * Params:
 * Return 0 if OK, something else if error.
 */
uint8_t WH_send (void *data, MessageInfo *mi);

/* Name WH_recv_blk
 * TODO
 * Params:
 * Return the number of bytes readed, 0 if ERROR or none.
 */
uint32_t WH_recv_blk (void **data, MessageInfo **mi, uint16_t num);

/* Name WH_send_blk
 * TODO
 * Params:
 * Return 0 if OK, something else if error.
 */
uint8_t WH_send_blk (void **data, MessageInfo **mi, uint16_t num);


#endif
