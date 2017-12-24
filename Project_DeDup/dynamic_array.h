#pragma once
#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "comdef.h"

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

uint32 dynamic_array_get(PDynamic_array head, uint32 index) {
	PDynamic_array curr_array = head;
	uint32 curr_index = index;
	while (curr_index > curr_array->length)
	{
		curr_array = curr_array->next_arr;
		curr_index -= curr_array->length;
	}
	return curr_array->arr[curr_index];
}

#endif // !DYNAMIC_ARRAY_H
