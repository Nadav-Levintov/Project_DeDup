#include "block_with_container.h"

Dedup_Error_Val private_allocate_block_with_container_pool_node(PBlock_with_container_pool_node pnode, uint32 size)
{

	pnode->next_empty_index = 0;
	pnode->array = (PBlock_with_container) malloc(sizeof(Block_with_container)*size);
	if(pnode->array == NULL)
	{
		return ALLOCATION_FAILURE;
	}
	pnode->pool_size = size;
	pnode->next = NULL;
	return SUCCESS;
}

Dedup_Error_Val block_with_container_pool_init(PBlock_with_container_pool pPool)
{
	Dedup_Error_Val res;
	assert(NULL != pPool);

	/*Create PBlock_with_container_pool_node and allocate the first array*/
	PBlock_with_container_pool_node pnode = (PBlock_with_container_pool_node) malloc(sizeof(Block_with_container_pool_node));
	if(pnode == NULL)
	{
		return ALLOCATION_FAILURE;
	}
	res = private_allocate_block_with_container_pool_node(pnode,
			BLOCK_CONTAINER_POOL_INITIAL_SZIZE);
	if(res == ALLOCATION_FAILURE)
	{
		free(pnode);
		return res;
	}
	/*Insert the new node to the pool*/
	pPool->current = pnode;
	pPool->head = pnode;
	pPool->pool_amount = 1;
	return SUCCESS;
}

Dedup_Error_Val block_with_container_pool_alloc(
		PBlock_with_container_pool pPool,
		uint32 size,
		PBlock_with_container *arr)
{
	assert(NULL != pPool);
	assert(0 < size);
	assert(NULL != arr);

	Dedup_Error_Val res;

	/*Need to allocate new pool node*/
	if ((pPool->current->pool_size - pPool->current->next_empty_index) < size)
	{
		PBlock_with_container_pool_node pnode =
				malloc(sizeof(Block_with_container_pool_node));

		if(pnode == NULL)
		{
			return ALLOCATION_FAILURE;
		}
		res = private_allocate_block_with_container_pool_node(
				pnode, BLOCK_CONTAINER_POOL_INITIAL_SZIZE);

		if(res == ALLOCATION_FAILURE)
		{
			free(pnode);
			return res;
		}
		pPool->current->next_empty_index = 0;
		pPool->current = pnode;
		pPool->pool_amount += 1;

		assert(pPool->current->pool_size >= size);
	}
	arr = (PBlock_with_container*)&(pPool->current->array[pPool->current->next_empty_index]);
	pPool->current->next_empty_index += size;
	return SUCCESS;
}

Dedup_Error_Val block_with_container_pool_destroy(PBlock_with_container_pool pool)
{
	if(pool == NULL)
	{
		return SUCCESS;
	}
	PBlock_with_container_pool_node pNode = pool->head;

	while(NULL != pNode)
	{
		free(pNode->array);
		pNode = pNode->next;
		free(pNode);
	}
	pool->pool_amount = 0;
	pool->head = NULL;
	pool->current = NULL;

	return SUCCESS;
}
