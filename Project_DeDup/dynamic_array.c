#include "dynamic_array.h"

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

Dedup_Error_Val dynamic_array_add(PDynamic_array head,PMemory_pool pool, uint32 val) {
	PDynamic_array curr_array = head;
	uint32 curr_index = curr_array->length - 1;

	while (curr_index > DYNAMIC_ARRAY_SIZE)
	{
		if (!curr_array->next_arr)
		{
			uint32 *new_arr;
			memory_pool_alloc(pool, sizeof(Dynamic_array), &new_arr);
			curr_array->next_arr = (PDynamic_array)new_arr;
			memset(curr_array->next_arr, 0, sizeof(Dynamic_array));
		}
		curr_array = curr_array->next_arr;
		curr_index = curr_array->length;
	}

	curr_array->arr[curr_index] = val;
	curr_array->length++;

	return SUCCESS;
}