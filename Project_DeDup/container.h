#pragma once
#ifndef CONTAINER_H
#define CONTAINER_H

#include "comdef.h"
#include "dynamic_array.h"
#include "dedup_file.h"

typedef struct container_file_t
{
	PDedup_File dedup_file;
	uint32 print_gen;
} Container_file, *PContainer_file;

typedef struct container_t
{
	uint32 sn;
	uint32 size; //in bytes
	uint32 num_of_files_using;
	Dynamic_array file_array; //contains sn of files
	uint32 num_of_blocks;
	Dynamic_array block_array; //contains sn of blocks
} Container, *PContainer;


Dedup_Error_Val container_add_file(PContainer container, PMemory_pool pool, uint32 file_sn);
Dedup_Error_Val container_add_block(PContainer container, PMemory_pool pool, uint32 block_sn, uint32 block_size);
Dedup_Error_Val container_del_file(PContainer container, uint32 file_sn);
Dedup_Error_Val container_del_block(PContainer container, uint32 block_sn, uint32 block_size);

typedef struct container_dynamic_array_t
{
	uint32 length;
	Container arr[DYNAMIC_ARRAY_SIZE];
	struct container_dynamic_array_t* next_arr;
}Container_dynamic_array, *PContainer_dynamic_array;

Dedup_Error_Val  container_dynamic_array_get(PContainer_dynamic_array head, uint32 index, PContainer* res);

Dedup_Error_Val  container_dynamic_array_add_and_get(PContainer_dynamic_array head, PMemory_pool pool, PContainer* res);

#endif // !CONTAINER_H
