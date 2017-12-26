#pragma once
#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H
#include "comdef.h"

typedef struct memory_pool_t
{
	uint32 next_free_index;
	uint32 next_free_pool_index;
	uint32 arr[POOL_INITIAL_SIZE];
	struct memory_pool_t* next_pool;
}Memory_pool, *PMemory_pool;

Dedup_Error_Val memory_pool_init(PMemory_pool pool);
Dedup_Error_Val memory_pool_alloc(PMemory_pool pool,uint32 size, uint32** res);
Dedup_Error_Val memory_pool_destroy(PMemory_pool *pool);

#endif // !MEMORY_POOL_H
