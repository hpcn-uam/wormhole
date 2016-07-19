#ifndef __WORM_H__
#define __WORM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// Usable defines:
//#define _WORMLIB_DEBUG_
//#define _DYM_ROUTE_DEBUG_
//#define _WORMLIB_STATISTICS_

	enum DataType {
		CUSTOM = -3, COMPOUND, STRING, ARRAY, INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64
	};

	typedef struct {
		enum DataType type;
		union {
			enum DataType arrayType;
			/*TODO:		struct {
						uint32_t size;
						struct ConnectionDataType *elementTypes;
					} compoundType;*/
		} ext ; //extended
	} ConnectionDataType;

	typedef struct {
		uint_fast32_t size;
		uint_fast32_t hash;
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

#ifdef _WORMLIB_STATISTICS_
	typedef struct {
		uint64_t totalIO;
		uint64_t lastIO;
		uint64_t lastCheck; //hptl_t
	} ConnectionStatistics;
#endif

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

	/** WH_printmsg
	 * Sends a message to Einstein and print it.
	 * The msg can be NULL
	 * @return 0 if OK, something else if error.
	 */
	uint8_t WH_printmsg(char *msg);

	/** WH_abort
	 * Sends a message to Einstein, print it and halt all worms.
	 * The msg can be NULL
	 * @return 0 if OK, something else if error.
	 */
	uint8_t WH_abort(char *msg);

	/** WH_recv
	 * TODO
	 * Params:
	 * @return the number of bytes readed, 0 if ERROR or none.
	 */
	uint32_t WH_recv(void *data, MessageInfo *mi);

	/** WH_send
	 * TODO
	 * Params:
	 * @return 0 if OK, something else if error.
	 */
	uint8_t WH_send(const void *const data, const MessageInfo *const mi);

	/** WH_flushIO
	 * Flushes all the IO queues.
	 * @return 0 if OK, something else if error.
	 */
	uint8_t WH_flushIO(void);

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
	//uint32_t WH_recv_blk(BulkList *bl);

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


#ifdef __cplusplus
}
#endif
#endif
