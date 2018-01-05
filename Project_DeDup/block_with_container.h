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
	PBlock_with_container_pool_node head;
	PBlock_with_container_pool_node current;
} Block_with_container_pool, *PBlock_with_container_pool;

Dedup_Error_Val block_with_container_pool_init(PBlock_with_container_pool pool);
Dedup_Error_Val block_with_container_pool_alloc(PBlock_with_container_pool pool, uint32 size, PBlock_with_container* arr);
Dedup_Error_Val block_with_container_pool_destroy(PBlock_with_container_pool pool);



#endif // !BLOCK_WITH_CONTAINER_H
