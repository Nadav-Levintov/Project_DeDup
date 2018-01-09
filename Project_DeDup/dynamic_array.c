#include "dynamic_array.h"

Dedup_Error_Val dynamic_array_get(PDynamic_array head, uint32 index, uint32* res) {
	PDynamic_array curr_array = head;
	uint32 curr_index = index;

	while (curr_index > curr_array->length - 1)
	{
		if (!curr_array->next_arr)
		{
			return DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR;
		}
		curr_index -= curr_array->length;
		curr_array = curr_array->next_arr;
	}

	*res = curr_array->arr[curr_index];

	return SUCCESS;
}

Dedup_Error_Val dynamic_array_update(PDynamic_array head, uint32 index, uint32 val) {
	PDynamic_array curr_array = head;
	uint32 curr_index = index;

	while (curr_index > curr_array->length - 1)
	{
		if (!curr_array->next_arr)
		{
			return DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR;
		}
		curr_index -= curr_array->length;
		curr_array = curr_array->next_arr;
	}

	curr_array->arr[curr_index] = val;

	return SUCCESS;
}

Dedup_Error_Val dynamic_array_add(PDynamic_array head, PMemory_pool pool, uint32 val) {
	PDynamic_array curr_array = head;
	uint32 curr_index = curr_array->length;

	while (curr_index > DYNAMIC_ARRAY_SIZE - 1)
	{
		if (!curr_array->next_arr)
		{
			uint32 *new_arr;
			memory_pool_alloc(pool, sizeof(Dynamic_array), &new_arr);
			assert(new_arr);
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

bool dynamic_array_contains(PDynamic_array head, uint32 val, uint32 * index)
{
	PDynamic_array curr_array = head;
	uint32 curr_index = 0, curr_array_index = 0;

	while (curr_array)
	{
		for (curr_index = 0; curr_index < DYNAMIC_ARRAY_SIZE; curr_index++)
		{
			if (curr_array->arr[curr_index] == val)
			{
				*index = curr_index + (curr_array_index * DYNAMIC_ARRAY_SIZE);
				return true;
			}

		}

		curr_array = curr_array->next_arr;
		curr_array_index++;
	}

	return false;
}
