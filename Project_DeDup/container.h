#pragma once
#ifndef CONTAINER_H
#define CONTAINER_H

#include "comdef.h"
#include "block.h"
#include "dedup_file.h"

typedef struct container_file_t
{
	PDedup_File file;
	uint32 print_gen;
} Container_file_t, *PCntainer_file_t;

typedef struct container_t
{
	uint32 sn;
	uint32 size;
	uint32 num_of_files_using;
	PFile_dynamic_array file_array;
	uint32 num_of_blocks;
	//TODO: check the issue with: PBlock_dynamic_array
	struct block_dynamic_array_t* block_array;
} Container, *PContainer;

typedef struct container_with_ref_count_t
{
	uint32 container_sn;
	uint32 ref;
} Container_with_ref_count, *PCntainer_with_ref_count;

typedef struct container_dynamic_array_t
{
	uint32 size;
	PContainer arr;
	struct container_dynamic_array_t* next_arr;
}Container_dynamic_array, *PContainer_dynamic_array;

typedef struct Container_with_ref_count_dynamic_array_t
{
	uint32 length;
	uint32 ref_count_arr[DYNAMIC_ARRAY_SIZE];
	PContainer arr[DYNAMIC_ARRAY_SIZE];
	struct Container_with_ref_count_dynamic_array_t* next_arr;
}Container_with_ref_count_dynamic_array, *PContainer_with_ref_count_dynamic_array;



#endif // !CONTAINER_H
