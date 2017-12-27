#pragma once
#ifndef BLOCK_H
#define BLOCK_H

#include "comdef.h"
#include "dynamic_array.h"

#define BLOCK_NOT_IN_CONTAINER (0xFFFFFFFFU)

typedef struct block_t
{
	uint32 sn;
	char id[ID_LENGTH];
	uint32 size;
	uint32 shared_by_files;
	Dynamic_array container_with_ref_count_array; // even indexes = container SN ; odd indexes = ref count
	uint32 last_container_sn;
	uint32 last_container_ref_count;
} Block, *PBlock;

Dedup_Error_Val block_init(PBlock block, uint32 sn,  char * id,  uint32 shared_by_files);
Dedup_Error_Val block_add_container(PBlock block, PMemory_pool pool, uint32 container_sn);
Dedup_Error_Val block_advance_last_container_ref_count(PBlock block);
Dedup_Error_Val block_get_container_sn_and_ref_count(PBlock block, uint32* container_sn, uint32* ref_count);

#endif // !BLOCK_H
