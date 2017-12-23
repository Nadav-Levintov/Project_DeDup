#pragma once
#ifndef DEDUP_DATA_SET_H
#define DEDUP_DATA_SET_H

#include <string.h>
#include "comdef.h"
#include "mem_mgr.h"
#include "dedup_file.h"
#include "block.h"
#include "container.h"
#include "block_with_container.h"

typedef struct dedup_data_set_t
{
	bool is_block_file; //Blocks = TRUE, physical = FALSE
	uint32 num_of_files;
	uint32 num_of_blocks;
	PDedup_File file_arr;
	PBlock block_arr;
	PContainer_dynamic_array container_arr;
	uint32 max_distance_between_containers_for_file;
	uint32 max_pointers_to_block;
	uint32 system_file_index[MAX_SYSTEMS];
	bool system_active[MAX_SYSTEMS];

	PBlock_with_container_pool block_with_container_pool;
} Dedup_data_set, *PDedup_data_set;

Dedup_Error_Val dedup_data_set_init_args(PDedup_data_set data_set, uint32 max_distance, uint32 max_pointers);
Dedup_Error_Val dedup_data_set_init_arrays(PDedup_data_set data_set, uint32 num_of_files, uint32 num_of_blocks);
Dedup_Error_Val dedup_data_set_add_file(PDedup_data_set data_set, char* line, uint32 length);
Dedup_Error_Val dedup_data_set_add_block(PDedup_data_set data_set, char* line, uint32 length);
Dedup_Error_Val dedup_data_set_set_block_size(PDedup_data_set data_set, uint32 block_sn, uint32 block_size);
Dedup_Error_Val dedup_data_set_analyze_to_containers(PDedup_data_set data_set);
Dedup_Error_Val dedup_data_set_delete_system(PDedup_data_set data_set, uint32 system_sn);
Dedup_Error_Val dedup_data_set_print_active_systems(PDedup_data_set data_set);

#endif // !DEDUP_DATA_SET_H