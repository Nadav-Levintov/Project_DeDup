#include "container.h"

Dedup_Error_Val container_add_file(PContainer container, PMemory_pool pool, uint32 file_sn)
{
	Dedup_Error_Val ret_val = SUCCESS;
	PDynamic_array arr = &(container->file_array);
	uint32 index;
	bool containes = dynamic_array_contains(arr, file_sn, &index);
	
	/* check if the file is already in the container, if not - add it */
	if (!containes)
	{
		ret_val = dynamic_array_add(arr, pool, file_sn);
		assert(ret_val == SUCCESS);
		container->num_of_files_using++;
	}
	
	return ret_val;
}

Dedup_Error_Val container_add_block(PContainer container, PMemory_pool pool, uint32 block_sn, uint32 block_size)
{
	Dedup_Error_Val ret_val = SUCCESS;
	PDynamic_array arr = &(container->block_array);
	uint32 index;
	bool containes = dynamic_array_contains(arr, block_sn, &index);
	
	/* check if the block is already in the container, if not - add it and update the container size */
	if (!containes)
	{
		ret_val = dynamic_array_add(arr, pool, block_sn);
		assert(ret_val == SUCCESS);
		container->num_of_blocks++;
		container->size += block_size;
	}

	return ret_val;
}

Dedup_Error_Val container_del_file(PContainer container, uint32 file_sn)
{
	PDynamic_array file_array = &(container->file_array);
	uint32 file_index_in_container;
	Dedup_Error_Val ret_val = SUCCESS;

	/* check if the file is in the container, if it is - delete it, else - do nothing */
	if (dynamic_array_contains(file_array, file_sn, &file_index_in_container))
	{
		ret_val = dynamic_array_update(file_array, file_index_in_container, REMOVED_SN);
		container->num_of_files_using--;
		assert(ret_val == SUCCESS);
	}

	return SUCCESS;
}

Dedup_Error_Val container_del_block(PContainer container, uint32 block_sn, uint32 size)
{
	PDynamic_array block_array = &(container->block_array);
	uint32 block_index_in_container;
	Dedup_Error_Val ret_val = SUCCESS;
	
	/* check if the block is in the container, if it is - delete it */
	if (dynamic_array_contains(block_array, block_sn, &block_index_in_container))
	{
		ret_val = dynamic_array_update(block_array, block_index_in_container, REMOVED_SN);
		assert(ret_val == SUCCESS);
	}
	
	/* We may have had this block in the container more than once so we update size and num_of_blocks again  - IS THIS CORRECT?*/
	container->num_of_blocks--;
	container->size -= size;

	return SUCCESS;
}

Dedup_Error_Val container_dynamic_array_get(PContainer_dynamic_array head, uint32 index, PContainer* res)
{
	PContainer_dynamic_array curr_array = head;
	uint32 curr_index = index;

	/* Iterate over the dynamic array nodes until the correct node based on the index */
	while (curr_index > curr_array->length - 1)
	{
		if (!curr_array->next_arr)
		{
			/* No array to continue to, we are out of bounds */
			return DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR;
		}
		curr_index -= curr_array->length;
		curr_array = curr_array->next_arr;
	}

	*res = &(curr_array->arr[curr_index]);

	return SUCCESS;
}

Dedup_Error_Val container_dynamic_array_add_and_get(PContainer_dynamic_array head, PMemory_pool pool, PContainer* res)
{
	PContainer_dynamic_array curr_array = head;
	uint32 curr_index = curr_array->length;
	uint32 curr_arry_index = 0;

	/* Iterate over the dynamic array nodes until the last node based on the index */
	while (curr_index > DYNAMIC_ARRAY_SIZE - 1)
	{
		if (!curr_array->next_arr)
		{
			/* Need to allocate a new array node */
			uint32 *new_arr;
			memory_pool_alloc(pool, sizeof(Container_dynamic_array), &new_arr);
			curr_array->next_arr = (PContainer_dynamic_array)new_arr;
			memset(curr_array->next_arr, 0, sizeof(Container_dynamic_array));
		}

		curr_array = curr_array->next_arr;
		curr_index = curr_array->length;
		curr_arry_index++;
	}

	/* update the cell as taken and the array */
	curr_array->arr[curr_index].sn = (curr_arry_index*DYNAMIC_ARRAY_SIZE) + curr_array->length;
	*res = &(curr_array->arr[curr_index]);
	curr_array->length++;

	return SUCCESS;
}
