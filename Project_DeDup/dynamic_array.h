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

Dedup_Error_Val dynamic_array_get(PDynamic_array head, uint32 index, uint32* res) {
	PDynamic_array curr_array = head;
	uint32 curr_index = index;

	while (curr_index > curr_array->length)
	{
		if (!curr_array->next_arr)
		{
			return DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR;
		}
		curr_array = curr_array->next_arr;
		curr_index -= curr_array->length;
	}

	*res = curr_array->arr[curr_index];

	return SUCCESS;
}

Dedup_Error_Val dynamic_array_update(PDynamic_array head, uint32 index, uint32 val) {
	PDynamic_array curr_array = head;
	uint32 curr_index = index;

	while (curr_index > curr_array->length)
	{
		if (!curr_array->next_arr)
		{
			return DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR;
		}
		curr_array = curr_array->next_arr;
		curr_index -= curr_array->length;
	}

	curr_array->arr[curr_index] = val;

	return SUCCESS;
}

Dedup_Error_Val dynamic_array_add(PDynamic_array head, uint32 val) {
	PDynamic_array curr_array = head;
	uint32 curr_index = curr_array->length - 1;

	while (curr_index + 1 >= DYNAMIC_ARRAY_SIZE)
	{
		if (!curr_array->next_arr)
		{
			//TODO: add allocation of next array.
		}
		curr_array = curr_array->next_arr;
		curr_index = curr_array->length;
	}

	curr_array->arr[curr_index] = val;
	curr_array->length++;

	return SUCCESS;
}

#endif // !DYNAMIC_ARRAY_H
