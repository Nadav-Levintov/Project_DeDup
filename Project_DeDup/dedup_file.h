#pragma once
#ifndef DEDUP_FILE_H
#define DEDUP_FILE_H

#include "comdef.h"
#include "avltree.h"
#include <stdio.h>

typedef struct dedup_file_t
{
	uint32 sn;
	uint32 dir_sn;
	uint32 block_amount;
	//PBlock_with_container block_with_container_array;
	avltree block_with_container_tree;
	avltree container_tree;
	char id[ID_LENGTH];
	uint32 sys_num;
} Dedup_File, *PDedup_File;

typedef struct block_with_container_t
{
	uint32 block_sn;
	uint32 container_sn;
} Block_with_container, *PBlock_with_container;

/*
	@Function:	dedup_file_init
	
	@Params:	file -	Pointer to the file struct to initialize.
				sn -	File SN.
				sys_num -	Number of the system the file belongs to.
				id -	ID of the file.
				dir_sn -	The serial number of the dir the file is in.
				block_amount -	The number of blocks in the file.
				block_with_container_array -	Pointer to the dynamic array for blocks and containers.
	
	@Desc:		Initialize the provided file with the given arguments
*/
Dedup_Error_Val dedup_file_init(PDedup_File file, uint32 sn, uint32 sys_num, char* id, uint32 dir_sn,
 uint32 block_amount);

/*
	@Function:	dedup_file_init
	
	@Params:	curr_file -	Pointer to the file struct to initialize.
				block_sn -	Block SN to check.
				max_index - The maximum index in the array to check.
	
	@Desc:		Check if the block with block_sn is in file blocks array before max_index.

	@Return:	true - if block is in the array.
				false - else.
*/
bool dedup_file_contains_current_block(PDedup_File curr_file, uint32 block_sn, uint32 max_index);

#endif // !DEDUP_FILE_H
