#include "memory_pool.h"

Dedup_Error_Val memory_pool_init(PMemory_pool pool)
{
	memset(pool, 0, sizeof(Memory_pool));
	return SUCCESS;
}

Dedup_Error_Val memory_pool_alloc(PMemory_pool pool, uint32 size, uint32 ** res)
{
	uint32 size_in_uint32_uints = (size / sizeof(uint32));
	uint32 size_of_uint32_to_alloc = (size % sizeof(uint32)) ? size_in_uint32_uints  + 1 : size_in_uint32_uints;
	PMemory_pool pool_to_alloc_from = pool;
	uint32 pool_to_alloc_from_index = pool->next_free_pool_index;

	assert(size < POOL_INITIAL_SIZE);

	for (uint32 i = 0; i < pool_to_alloc_from_index; i++)
	{
		pool_to_alloc_from = pool_to_alloc_from->next_pool;
	}

	if (POOL_INITIAL_SIZE >= (pool->next_free_index + size_of_uint32_to_alloc))
	{
		pool_to_alloc_from->next_pool = malloc(sizeof(Memory_pool));
		if (!pool_to_alloc_from->next_pool)
		{
			return ALLOCATION_FAILURE;
		}
		pool->next_free_index = 0;
		pool->next_free_pool_index++;
		pool_to_alloc_from = pool_to_alloc_from->next_pool;
	}

	*res = &(pool_to_alloc_from->arr[pool->next_free_index]);
	pool->next_free_index += size_of_uint32_to_alloc;

	return SUCCESS;
}

Dedup_Error_Val memory_pool_destroy(PMemory_pool *pool)
{
	PMemory_pool next_pool = NULL;
	PMemory_pool pool_to_free = (*pool)->next_pool;

	while (pool_to_free)
	{
		next_pool = pool_to_free->next_pool;
		free(pool_to_free);
		pool_to_free = next_pool;
	}

	free(*pool);
	*pool = NULL;

	return SUCCESS;
}
