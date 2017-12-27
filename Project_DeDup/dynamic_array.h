#pragma once
#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "comdef.h"
#include "memory_pool.h"

typedef struct dynamic_array_t
{
	uint32 length;
	uint32 arr[DYNAMIC_ARRAY_SIZE];
	struct dynamic_array_t* next_arr;
}Dynamic_array, *PDynamic_array;

Dedup_Error_Val dynamic_array_get(PDynamic_array head, uint32 index, uint32* res);

Dedup_Error_Val dynamic_array_update(PDynamic_array head, uint32 index, uint32 val);

Dedup_Error_Val dynamic_array_add(PDynamic_array head, PMemory_pool pool,  uint32 val);


#endif // !DYNAMIC_ARRAY_H
