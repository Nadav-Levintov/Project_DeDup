#pragma once
#ifndef CONTAINER_H
#define CONTAINER_H

#include "comdef.h"
#include "dynamic_array.h"
#include "dedup_file.h"

typedef struct container_file_t
{
	uint32 print_gen;
	PDedup_File dedup_file;
} Container_file, *PContainer_file;

typedef struct container_t
{
	uint32 sn;
	uint32 size; //in bytes
	uint32 num_of_files_using;
	uint32 num_of_blocks;
	Dynamic_array file_array; //contains sn of files
	Dynamic_array block_array; //contains sn of blocks
} Container, *PContainer;

/*
	@Function: container_add_file
	
	@Params:	container -	Pointer to current container.
				pool -		Pointer to pool to allocate memory from if needed.
				file_sn -	Serial number of file to add.
	
	@Desc:		Add the file serial number to the container dynamic array, if needed allocate memory to extend the array.
*/
Dedup_Error_Val container_add_file(PContainer container, PMemory_pool pool, uint32 file_sn);

/*
	@Function: container_add_block
	
	@Params:	container -		Pointer to current container.
				block_sn -		Pointer to pool to allocate memory from if needed.
				block size -	Serial number of block to add.
	
	@Desc:		Add the block serial number to the container dynamic array, if needed allocate memory to extend the array.
*/
Dedup_Error_Val container_add_block(PContainer container, PMemory_pool pool, uint32 block_sn, uint32 block_size);

/*
	@Function: container_del_file
	
	@Params:	container -	Pointer to current container.
				file_sn -	Serial number of the file to remove.
	
	@Desc:		Remove the file serial number from container dynamic array.
*/
Dedup_Error_Val container_del_file(PContainer container, uint32 file_sn);

/*
	@Function: container_del_block
	
	@Params:	container -		Pointer to current container.
				block_sn -		Serial number of the block to remove.
				block_size -	Size of the block to remove
	
	@Desc:		Remove the block serial number from container dynamic array.
*/
Dedup_Error_Val container_del_block(PContainer container, uint32 block_sn, uint32 block_size);

typedef struct container_dynamic_array_t
{
	uint32 length;
	struct container_dynamic_array_t* next_arr;
	Container arr[DYNAMIC_ARRAY_SIZE];
}Container_dynamic_array, *PContainer_dynamic_array;

/*
	@Function: container_dynamic_array_get
	
	@Params:	head -	Pointer to head of the dynamic array.
				index -	Requested index of the container.
				res -	Pointer which will hold the pointer to the requested container.
	
	@Desc:		Return a pointer to the container at the spcecified index.
*/
Dedup_Error_Val  container_dynamic_array_get(PContainer_dynamic_array head, uint32 index, PContainer* res);

/*
	@Function: container_dynamic_array_add_and_get
	
	@Params:	head -	Pointer to head of the dynamic array.
				pool -	Pointer to pool to allocate memory from if needed.
				res -	Pointer which will hold the pointer to the added container.
	
	@Desc:		Add a new initialized container to the array and return a pointer to it.
*/
Dedup_Error_Val  container_dynamic_array_add_and_get(PContainer_dynamic_array head, PMemory_pool pool, PContainer* res);

#endif // !CONTAINER_H
