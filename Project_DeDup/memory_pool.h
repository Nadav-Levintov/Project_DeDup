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

/*
	@Function:	memory_pool_init
	
	@Params:	pool -	Pointer to the pool struct to initialize.
	
	@Desc:		Pool struct will be initialized, allocated and the memory would be set to 0.
*/
Dedup_Error_Val memory_pool_init(PMemory_pool pool);

/*
	@Function:	memory_pool_alloc
	
	@Params:	pool -	Pointer to the pool from which to allocate memory from.
				size -	The size (in bytes) of memory to allocate.
				res -	Pointer to the start of the allocated memory.
	
	@Desc:		Memory of the requested size will be allocated from the pool, if required the pool would be extended.
				A pointer to the begining of the memory would be place in the res pointer.
*/
Dedup_Error_Val memory_pool_alloc(PMemory_pool pool,uint32 size, uint32 **res);

/*
	@Function:	memory_pool_destroy
	
	@Params:	pool -	Pointer to the pool to destroy
	
	@Desc:		All memory of the pool will be freed.
*/
Dedup_Error_Val memory_pool_destroy(PMemory_pool pool);

#endif // !MEMORY_POOL_H
