#pragma once
#ifndef BLOCK_H
#define BLOCK_H

#include "comdef.h"
#include "container.h"

typedef struct block_t
{
	uint32 sn;
	char id[ID_LENGTH];
	uint32 size;
	uint32 num_of_files_using;
	Container_with_ref_count_dynamic_array cwrc_array;
	uint32 last_container_sn;
} Block, *PBlock;

Dedup_Error_Val block_create(PBlock block, uint32 sn,  char * id,  uint32 shared_by_files);
Dedup_Error_Val block_set_size(PBlock block, uint32 block_size);

typedef struct block_dynamic_array_t
{
	uint32 size;
	uint32 sn_arr;
	struct block_dynamic_array_t* next_arr;
}Block_dynamic_array, *PBlock_dynamic_array;


#endif // !BLOCK_H
