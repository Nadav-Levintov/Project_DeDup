#pragma once
#ifndef BLOCK_WITH_CONTAINER_H
#define BLOCK_WITH_CONTAINER_H

#include "comdef.h"

typedef struct block_with_containers_t
{
	uint32 block_sn;
	uint32 container_sn;
}Block_with_container, *PBlock_with_container;


typedef struct block_with_container_pool_node_t
{
	struct block_with_container_pool_node_t* next;
	uint32 pool_size;
	uint32 next_empty_index;
	PBlock_with_container array;
} Block_with_container_pool_node, *PBlock_with_container_pool_node;

typedef struct block_with_container_pool_t
{
	uint32 pool_amount;
	Block_with_container_pool_node head;
	PBlock_with_container_pool_node current;
} Block_with_container_pool, *PBlock_with_container_pool;

/*
	@Function:	block_with_container_pool_init

	@Params:	Pointer to a pool to initialize.

	@Desc:		Initialize the pool and allocate it's initial memory.
*/
Dedup_Error_Val block_with_container_pool_init(PBlock_with_container_pool pool);

/*
	@Function:	block_with_container_pool_alloc

	@Params:	Pointer to a pool to allocate memory from.
				Requested size to allocate.
				Pointer to the allocated memory will be returend in arr.

	@Desc:		Allocate a block with container array of the given size from the specifed memory pool.
				The allocted address is return in the arr pointer.
*/
Dedup_Error_Val block_with_container_pool_alloc(PBlock_with_container_pool pool, uint32 size, PBlock_with_container* arr);

/*
	@Function:	block_with_container_pool_destroy
	
	@Params:	Pointer to a pool to free.
	
	@Desc:		Free all memory of the specified pool.
*/
Dedup_Error_Val block_with_container_pool_destroy(PBlock_with_container_pool pool);

#endif // !BLOCK_WITH_CONTAINER_H
