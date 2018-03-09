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

/*
	@Function:	block_init
	
	@Params:	Pointer to block to initialize
				Block serial number
				Block ID
				Number of files which share the block

	@Desc:		Initialize a block struct with the provided values.
*/

Dedup_Error_Val block_init(PBlock block, uint32 sn,  char * id,  uint32 shared_by_files);
/*
	@Function:	block_add_container
	
	@Params:	Pointer to block
				Pointer to the memory pool to allocate storage from
				Continer seriale number
	
	@Desc:		Add the provided serial number to the block continers array.
				The continer with the provided SN contins the received block.
				If the container already in the array increase its refernce number by 1.
				
*/
Dedup_Error_Val block_add_container(PBlock block, PMemory_pool pool, uint32 container_sn);

/*
	@Function:	block_advance_last_container_ref_count

	@Params:	Pointer to block.

	@Desc:		Add 1 to the reference counter of the last container to add the block.
*/
Dedup_Error_Val block_advance_last_container_ref_count(PBlock block);

/*
	@Function:	container_with_ref_array_dynamic_array_contains

	@Params:	Pointer to the head of the dynamic array.
				Container SN to lookup in the array.
				Pointer which will hold the index of the value in the array
					if the value is found in the array.

	@Desc:		Look for val in the dynamic array starting with head, if found the index is returned.

	@Return:	True if the array contins val, False if not.
*/
bool container_with_ref_array_dynamic_array_contains(PDynamic_array head, uint32 val, uint32 * index);

/*
@Function:	block_container_decrease_ref_count

@Params:	Pointer to the block.
			Serial number of the continer to decrease it's reference count for the provided block.
			Pointer which will hold the updated refernce count.

@Desc:		Decrease the refernce count of for the current block in the continer with the provided serial number,
				if the continer is found the updated reference count is returned in the ref_count pointer, if not
				INDEX_NOT_FOUND is returned.
*/
Dedup_Error_Val block_container_decrease_ref_count(PBlock block, uint32 container_sn, uint32* ref_count);
#endif // !BLOCK_H
