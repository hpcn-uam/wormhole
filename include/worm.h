#ifndef __WORM_H__
#define __WORM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

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
		uint32_t size;
		uint32_t hash;
		uint16_t category;
		ConnectionDataType *type;
	} MessageInfo;

	typedef struct {
		size_t numInputTypes;
		ConnectionDataType *inputTypes;
	} WormConfig;

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

	/** WH_recv_blk
	 * Receives multiples messages.
	 * @return the number of bytes readed, 0 if ERROR or none.
	 */
	uint32_t WH_recv_blk(void **data, MessageInfo **mi, uint16_t num);

	/** WH_send_blk
	 * Sends multiples messages.
	 * @return 0 if OK, something else if error.
	 */
	uint8_t WH_send_blk(const void **const data, const MessageInfo **const mi, const uint16_t num);

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


#ifdef __cplusplus
}
#endif
#endif
