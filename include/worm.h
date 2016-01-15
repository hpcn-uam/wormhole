#ifndef __WORM_H__
#define __WORM_H__

#include "common.h"

enum {CUSTOM, COMPOUND, ARRAY, INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64, STRING} DataType;

struct {
    enum DataType type;
    union
{
        enum DataType arrayType;
        /*TODO:		struct
        		{
        			uint32_t size;
        			struct ConnectionDataType *elementTypes;
        		} compoundType;*/
    }
} ConnectionDataType;

typedef struct {
    uint16_t category;
    uint16_t priority;
    uint16_t dport;
    uint16_t did;
    uint32_t dip; //TODO fix para ipv6
    int socket;
    struct ConnectionDataType;
} Connection;


#endif
