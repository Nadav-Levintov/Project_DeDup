#pragma once
#ifndef DEDUP_FILE_H
#define DEDUP_FILE_H

#include "comdef.h"
#include "block_with_container.h"
#include <stdio.h>

typedef struct dedup_file_t
{
	uint32 sn;
	uint8 sys_num;
	char id[ID_LENGTH];
	uint32 dir_sn;
	uint32 block_amount;
	PBlock_with_container block_with_container_array;
} Dedup_File, *PDedup_File;

Dedup_Error_Val dedup_file_create(PDedup_File file, uint32 sn, uint8 sys_num, char* id, uint32 dir_sn, uint32 block_amount,
	PBlock_with_container block_with_container_array);

#endif // !DEDUP_FILE_H
