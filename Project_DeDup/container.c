#include "container.h"

int cmp_func(const void* cnt1, const void* cnt2)
{
	uint32 *uint1 = (uint32*) cnt1;
	uint32 *uint2 = (uint32*) cnt2;

	return *uint1 - *uint2;
}

void create_container(PContainer *pContainer, PMemory_pool pool, uint32 container_sn)
{
	Dedup_Error_Val res = SUCCESS;
	res = memory_pool_alloc(pool, sizeof(**pContainer), (uint32**)pContainer);
	assert(res == SUCCESS);
	(*pContainer)->sn = container_sn;
	avltree_init(cmp_func,&((*pContainer)->file_array));
	avltree_init(cmp_func,&((*pContainer)->block_array));
	(*pContainer)->num_of_blocks = 0;
	(*pContainer)->num_of_files_using = 0;
	(*pContainer)->size = 0;
}

Dedup_Error_Val container_add_file(PContainer container, PMemory_pool pool, uint32 file_sn)
{
	Dedup_Error_Val ret_val = SUCCESS;
	avltree* pTree = &(container->file_array);
	uint32 *pValue = NULL;
	uint32 *file_sn_alloc = NULL;
	
	ret_val = memory_pool_alloc(pool, sizeof(uint32), (uint32**)&file_sn_alloc);
	assert(ret_val == SUCCESS);

	*file_sn_alloc = file_sn;

	/* Add file to container file tree and increase num of files using */
	pValue = avltree_add(pTree, (void*)file_sn_alloc, pool);
	if (pValue == NULL)
	{
		container->num_of_files_using++;
	}

	return ret_val;
}

Dedup_Error_Val container_add_block(PContainer container, PMemory_pool pool, uint32 block_sn, uint32 block_size)
{
	Dedup_Error_Val ret_val = SUCCESS;
	avltree* pTree = &(container->block_array);
	uint32 *pValue = NULL;
	uint32 *block_sn_alloc = NULL;

	ret_val = memory_pool_alloc(pool, sizeof(uint32), (uint32**)&block_sn_alloc);
	assert(ret_val == SUCCESS);

	*block_sn_alloc = block_sn;

	/* Check if the array already holds container with the provided SN */
	pValue = avltree_add(pTree, (void*)block_sn_alloc, pool);

	
	/* check if the block is already in the container, if not - add it and update the container size */
	if (pValue == NULL)
	{	
		container->num_of_blocks++;
		container->size += block_size;
	}

	return ret_val;
}

Dedup_Error_Val container_del_file(PContainer container, uint32 file_sn)
{
	avltree* pTree = &(container->file_array);
	uint32 *pValue = NULL;

	/* Check if the array already holds container with the provided SN */
	pValue = avltree_remove(pTree, &file_sn);

	/* check if the file is in the container, if it is - delete it, else - do nothing */
	if (pValue != NULL)
	{
		container->num_of_files_using--;
	}

	return SUCCESS;
}

Dedup_Error_Val container_del_block(PContainer container, uint32 block_sn, uint32 size)
{
	avltree* pTree = &(container->block_array);
	uint32 *pValue = NULL;

	/* Check if the array already holds container with the provided SN */
	pValue = avltree_remove(pTree, &block_sn);

	
	/* check if the block is in the container, if it is - delete it, else do nothing.*/
	if (pValue != NULL)
	{
		container->num_of_blocks--;
		container->size -= size;
	}

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
