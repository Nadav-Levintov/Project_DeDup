#pragma once
#ifndef DEDUP_FILE_H
#define DEDUP_FILE_H

#include "comdef.h"
#include "block_with_container.h"

typedef struct dedup_file_t
{
	uint32 sn;
	uint8 sys_num;
	char id[ID_LENGTH];
	uint32 dir_sn;
	uint32 block_amount;
	PBlock_with_container bwc_array;
	uint32 container_amount;
} Dedup_File, *PDedup_File;

Dedup_Error_Val dedup_file_create(PDedup_File file, uint32 sn, uint8 sys_num, char* id, uint32 dir_sn, uint32 block_amount,
	PBlock_with_container bwc_array);

typedef struct file_dynamic_array_t
{
	uint32 length;
	uint32 sn_arr[DYNAMIC_ARRAY_SIZE];
	struct file_dynamic_array_t* next_arr;
}File_dynamic_array, *PFile_dynamic_array;


#endif // !DEDUP_FILE_H
