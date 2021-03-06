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
#ifndef __WORM_H__
#define __WORM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libworm/common.h>

// Usable defines:
//#define _WORMLIB_DEBUG_
//#define _DYM_ROUTE_DEBUG_

enum DataType {
	CUSTOM = -3,
	COMPOUND,
	STRING,
	ARRAY  = 0,
	INT8   = 1,
	CHAR   = 1,
	UINT8  = 2,
	UCHAR  = 2,
	INT16  = 3,
	JCHAR  = 3,
	UINT16 = 4,
	INT32,
	UINT32,
	INT64,
	UINT64
};

typedef struct {
	enum DataType type;
	union {
		enum DataType arrayType;
		/*TODO:		struct {
		            uint32_t size;
		            struct ConnectionDataType *elementTypes;
		        } compoundType;*/
	} ext;  // extended
} ConnectionDataType;

typedef struct {
	uint64_t size;
	uint64_t hash;
	ConnectionDataType const *type;
	uint_fast16_t category;
} MessageInfo;

typedef struct {
	size_t numInputTypes;
	ConnectionDataType *inputTypes;
} WormConfig;

typedef struct {
	MessageInfo *info;
	void *data;
} BulkMsg;

typedef struct {
	union {
		BulkMsg *msgs;
		void *rawdata;
	} list;
	size_t len;
	int_fast8_t cpyalign;
} BulkList;

/** WH_setup_types
 * Setups the available types of this Worm.
 * 	If this function is never called, only UINT8-Array would be supported.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_setup_types(size_t nTypes, ConnectionDataType *types);

/** WH_init
 * Starts the WormHole Library
 * @return 0 if OK, something else if error.
 */
uint8_t WH_init(void);

/** WH_halt
 * Stops and free the WormHole Library
 * @return 0 if OK, something else if error.
 */
uint8_t WH_halt(void);

/** WH_recv
 * Receives data from some one of the sources.
 * If data is an array and mi->size is not enougth for writing incomming data,
 * the function would return 0 and set errno to EMSGSIZE (Message too long)
 * If all the recvrs are disconnected, errno is set to ENOTCONN.
 * Params:
 * @return the number of bytes readed, 0 if ERROR or none.
 */
uint32_t WH_recv(void *restrict data, MessageInfo *restrict mi);

/** WH_send
 * TODO
 * Params:
 * @return 0 if OK, something else if error.
 */
uint8_t WH_send(const void *restrict const data, const MessageInfo *restrict const mi);

/** WH_connectionDataTypecmp
 * Compares 2 datatypes
 * @return 0 if are equal, something else if not.
 */
uint8_t WH_connectionDataTypecmp(const ConnectionDataType *const a, const ConnectionDataType *const b);

/**************** Info Utils ****************/

/** WH_get_id
 * @return the WORM-ID.
 */
uint16_t WH_get_id(void);

/**************** Bulk Utils ****************/

/** WH_recv_blk //TODO : not implemented
 * Receives multiples messages.
 * @return the number of bytes readed, 0 if ERROR or none.
 */
// uint32_t WH_recv_blk(BulkList *bl);

/** WH_send_blk
 * Sends multiples messages.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_send_blk(const BulkList *const bl);

/** WH_BL_create
 * @param cpyalign if set to other than 0, then each data added to the list would be copied and memory aligned
 * @return a new BulkList. Null if error.
 */
BulkList *WH_BL_create(const int_fast8_t cpyalign);

/** WH_BL_add
 * Adds a message info and msg to the list.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_BL_add(BulkList *bl, const void *const msg, const MessageInfo *const mi);

/** WH_BL_free
 * Frees the list
 */
void WH_BL_free(BulkList *bl);

/**************** Other Utils ****************/

/** WH_sprintf
 * Returns a malloced-string using classic printf format. Must be freeded
 * @return a malloced-string.
 */
char *WH_sprintf(const char *restrict format, ...);

#ifdef __cplusplus
}
#endif

#endif
