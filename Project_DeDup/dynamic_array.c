#include "dynamic_array.h"

uint32 dynamic_array_find_max(PDynamic_array head)
{
	uint32 max = 0;
	for (uint32 i = 0; i < head->length; i++)
	{
		if (max < head->arr[i])
			max = head->arr[i];
	}
	return max;
}

uint32 dynamic_array_find_min(PDynamic_array head)
{
	uint32 min = 0xFFFFFFFFU;
	for (uint32 i = 0; i < head->length; i++)
	{
		if (min > head->arr[i])
			min = head->arr[i];
	}
	return min;
}

Dedup_Error_Val dynamic_array_get(PDynamic_array head, uint32 index, uint32* res) {
	PDynamic_array curr_array = head;
	uint32 curr_index = index;

	/* Iterate over array nodes */
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

Dedup_Error_Val dynamic_array_update(PDynamic_array head, uint32 index, uint32 val, bool update_min_max) {
	PDynamic_array curr_array = head;
	uint32 curr_index = index;
	uint32 old_val;

	/* Iterate over array nodes */
	while (curr_index > curr_array->length - 1)
	{
		if (!curr_array->next_arr)
		{
			return DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR;
		}
		curr_index -= curr_array->length;
		curr_array = curr_array->next_arr;
	}

	old_val = curr_array->arr[curr_index];
	curr_array->arr[curr_index] = val;
	if (update_min_max) {
		if (old_val == curr_array->max_val)
		{
			curr_array->max_val = dynamic_array_find_max(curr_array);
		}

		if (old_val == curr_array->min_val)
		{
			curr_array->max_val = dynamic_array_find_min(curr_array);
		}
	}

	
	return SUCCESS;
}

Dedup_Error_Val dynamic_array_add(PDynamic_array head, PMemory_pool pool, uint32 val, bool update_min_max) {
	PDynamic_array curr_array = head;
	uint32 curr_index = curr_array->length;

	/* Iterate until an empty cell is found */
	while (curr_index > DYNAMIC_ARRAY_SIZE - 1)
	{
		if (!curr_array->next_arr)
		{
			/* If an empty cell is not found, allocate a new array node from pool */
			uint32 *new_arr;
			memory_pool_alloc(pool, sizeof(Dynamic_array), &new_arr);
			assert(new_arr);
			curr_array->next_arr = (PDynamic_array)new_arr;
			memset(curr_array->next_arr, 0, sizeof(Dynamic_array));
		}
		curr_array = curr_array->next_arr;
		curr_index = curr_array->length;
	}
	if (update_min_max) {
		if (curr_index == 0)
		{
			curr_array->max_val = val;
			curr_array->min_val = val;
		}
		else {
			if (val > curr_array->max_val)
			{
				curr_array->max_val = val;
			}
			if (val < curr_array->min_val)
			{
				curr_array->min_val = val;
			}
		}
	}
	curr_array->arr[curr_index] = val;
	curr_array->length++;

	return SUCCESS;
}

bool dynamic_array_contains(PDynamic_array head, uint32 val, uint32 * index)
{
	PDynamic_array curr_array = head;
	uint32 curr_index = 0, curr_array_index = 0;

	/* Iterate over array nodes */
	while (curr_array)
	{
		if (val <= curr_array->max_val && val >= curr_array->min_val)
		{
			for (curr_index = 0; curr_index < curr_array->length; curr_index++)
			{
				if (curr_array->arr[curr_index] == val)
				{
					*index = curr_index + (curr_array_index * DYNAMIC_ARRAY_SIZE);
					return true;
				}

			}
		}
		curr_array = curr_array->next_arr;
		curr_array_index++;
	}

	return false;
}
