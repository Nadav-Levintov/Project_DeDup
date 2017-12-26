#pragma once
#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "comdef.h"
#include "memory_pool.h"

/* can we make all of our dynamic arrays of the same type?
	most of them only store SN (uint32) or pointer to container (which is same size of uin32
	so we can use a cast but use the same struct for all.
	The only diff is container_wite_ref_count struct that has 2 uint32, ref count and pointer
	if we use the same struct all over it will reduce the amount of duplicated code becuase if we don't
	we'll need to copy the functions for all dynamic arrays.
	Maybe we will use 2 stucts, one specific for container_with_ref and the other for all else.
	*/

typedef struct dynamic_array_t
{
	uint32 length;
	uint32 arr[DYNAMIC_ARRAY_SIZE];
	struct dynamic_array_t* next_arr;
}Dynamic_array, *PDynamic_array;

Dedup_Error_Val dynamic_array_init(PDynamic_array head,PMemory_pool pool, uint32 index);

Dedup_Error_Val dynamic_array_get(PDynamic_array head, uint32 index, uint32* res);

Dedup_Error_Val dynamic_array_update(PDynamic_array head, uint32 index, uint32 val);

Dedup_Error_Val dynamic_array_add(PDynamic_array head, PMemory_pool pool,  uint32 val);


#endif // !DYNAMIC_ARRAY_H
