#include "block.h"

Dedup_Error_Val block_init(PBlock block, uint32 sn, char * id, uint32 shared_by_files)
{
	block->last_container_sn = BLOCK_NOT_IN_CONTAINER;
	block->last_container_ref_count = 0;
	block->sn = sn;
	strcpy(block->id, id);
	block->shared_by_files = shared_by_files;
	return SUCCESS;
}

Dedup_Error_Val block_add_container(PBlock block, PMemory_pool pool, uint32 container_sn)
{
	Dedup_Error_Val ret_val = SUCCESS;
	PDynamic_array arr = &(block->container_with_ref_count_array);
	uint32 index = 0;
	uint32 ref_count = 0;
	PDynamic_array container_with_ref_arr = &(block->container_with_ref_count_array);

	bool containes = container_with_ref_array_dynamic_array_contains(arr, container_sn, &index);
	if (!containes)
	{
		ret_val = dynamic_array_add(arr, pool, container_sn);
		assert(ret_val == SUCCESS);
		ret_val = dynamic_array_add(arr, pool, 1);
		assert(ret_val == SUCCESS);

		block->last_container_sn = container_sn;
		block->last_container_ref_count = 1;
	}else
	{
		ret_val = dynamic_array_get(container_with_ref_arr, index + 1, &ref_count);
		assert(ret_val == SUCCESS);
		ref_count++;
		ret_val = dynamic_array_update(container_with_ref_arr, index + 1, ref_count);
		assert(ret_val == SUCCESS);

		block->last_container_ref_count++;
	}
	return ret_val;
}

Dedup_Error_Val block_advance_last_container_ref_count(PBlock block)
{
	assert(block->container_with_ref_count_array.length);
	block->last_container_ref_count++;
	Dedup_Error_Val ret_val = dynamic_array_update(&(block->container_with_ref_count_array),
		block->container_with_ref_count_array.length - 1, block->last_container_ref_count);
	assert(ret_val == SUCCESS);
	
	return ret_val;
}

bool container_with_ref_array_dynamic_array_contains(PDynamic_array head, uint32 val, uint32 * index)
{
	PDynamic_array curr_array = head;
	uint32 curr_index = 0, curr_array_index = 0;

	while (curr_array)
	{
		for (curr_index = 0; curr_index < curr_array->length; curr_index+=2) //+2 because odd indexs are for refs
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

Dedup_Error_Val block_container_decrece_ref_count(PBlock block, uint32 container_sn, uint32* ref_count)
{
	Dedup_Error_Val ret = SUCCESS;
	uint32 container_index_in_arr = 0;
	PDynamic_array container_with_ref_arr = &(block->container_with_ref_count_array);
	if (container_with_ref_array_dynamic_array_contains(container_with_ref_arr, container_sn, &container_index_in_arr))
	{
		ret = dynamic_array_get(container_with_ref_arr, container_index_in_arr + 1, ref_count);
		assert(ret == SUCCESS);
		(*ref_count)--;
		ret = dynamic_array_update(container_with_ref_arr, container_index_in_arr + 1, *ref_count);
		assert(ret == SUCCESS);
	}
	else
	{
		*ref_count = INDEX_NOT_FOUND;
	}

	return SUCCESS;
}
