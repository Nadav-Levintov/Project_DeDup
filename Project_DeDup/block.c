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

	/* Check if the array already holds container with the provided SN */
	bool containes = container_with_ref_array_dynamic_array_contains(arr, container_sn, &index);
	if (!containes)
	{
		/* New container, add it and add a refernce counter in the next array cell */
		ret_val = dynamic_array_add(arr, pool, container_sn, true);
		assert(ret_val == SUCCESS);
		ret_val = dynamic_array_add(arr, pool, 1, false);
		assert(ret_val == SUCCESS);

		block->last_container_sn = container_sn;
		block->last_container_ref_count = 1;
	}else
	{
		/* Old container, get the current refernce counter and increase it by 1 */
		ret_val = dynamic_array_get(container_with_ref_arr, index + 1, &ref_count);
		assert(ret_val == SUCCESS);
		ref_count++;
		ret_val = dynamic_array_update(container_with_ref_arr, index + 1, ref_count, false);
		assert(ret_val == SUCCESS);

		block->last_container_ref_count++;
	}
	return ret_val;
}

Dedup_Error_Val block_advance_last_container_ref_count(PBlock block)
{
	assert(block->container_with_ref_count_array.length);

	PDynamic_array container_with_ref_arr = &(block->container_with_ref_count_array);

	/* Increase the ref count for the last continer to add the block */
	block->last_container_ref_count++;
	
	/* Update the continer with reference counter array accordingly */
	Dedup_Error_Val ret_val = dynamic_array_update(container_with_ref_arr,
		block->container_with_ref_count_array.length - 1, block->last_container_ref_count, false);
	
	assert(ret_val == SUCCESS);
	
	return ret_val;
}

bool container_with_ref_array_dynamic_array_contains(PDynamic_array head, uint32 container_sn, uint32 * index)
{
	PDynamic_array curr_array = head;
	uint32 curr_index = 0, curr_array_index = 0;

	/* Iterate over the dynamic array nodes until the requested container SN is found or the dynamic array ends*/
	while (curr_array)
	{
		if (container_sn >= curr_array->min_val && container_sn <= curr_array->max_val)
		{
			for (curr_index = 0; curr_index < curr_array->length; curr_index += 2) //+2 because odd indexs are for refs
			{
				if (curr_array->arr[curr_index] == container_sn)
				{
					/* Index = curreny array index + number of array nodes we passed * size of array nodes size */
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

Dedup_Error_Val block_container_decrease_ref_count(PBlock block, uint32 container_sn, uint32* ref_count)
{
	Dedup_Error_Val ret = SUCCESS;
	uint32 container_index_in_arr = 0;
	PDynamic_array container_with_ref_arr = &(block->container_with_ref_count_array);

	/* Find the continer in the array and update the ref count */
	if (container_with_ref_array_dynamic_array_contains(container_with_ref_arr, container_sn, &container_index_in_arr))
	{
		/* Get the ref count and decrease it */
		ret = dynamic_array_get(container_with_ref_arr, container_index_in_arr + 1, ref_count);
		assert(ret == SUCCESS);
		(*ref_count)--;
		ret = dynamic_array_update(container_with_ref_arr, container_index_in_arr + 1, *ref_count, false);
		assert(ret == SUCCESS);
	}
	else
	{
		*ref_count = INDEX_NOT_FOUND;
	}

	return SUCCESS;
}
