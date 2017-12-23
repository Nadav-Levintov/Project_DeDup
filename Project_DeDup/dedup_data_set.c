#pragma warning(disable : 4996)
#include "dedup_data_set.h"

Dedup_Error_Val dedup_data_set_init_args(PDedup_data_set data_set, uint32 max_distance, uint32 max_pointers)
{
	data_set->max_distance_between_containers_for_file = max_distance;
	data_set->max_pointers_to_block = max_pointers;

	return SUCCESS;
}

Dedup_Error_Val dedup_data_set_init_arrays(PDedup_data_set data_set, uint32 num_of_files, uint32 num_of_blocks)
{
	data_set->num_of_blocks = num_of_blocks;
	data_set->block_arr = malloc(sizeof(Block)*num_of_blocks);
	if (data_set->block_arr == NULL)
	{
		return ALLOCATION_FAILURE;
	}
	data_set->num_of_files = num_of_files;
	data_set->file_arr = malloc(sizeof(Dedup_File)*num_of_files);
	if (data_set->file_arr == NULL)
	{
		free(data_set->block_arr);
		return ALLOCATION_FAILURE;
	}

	return SUCCESS;
}
Dedup_Error_Val dedup_data_set_add_file(PDedup_data_set data_set, char* line, uint32 length)
{
	if (strcmp(strtok(line, ","), "F") != 0)
		return INVALID_ARGUMENT_FAILURE;

	Dedup_Error_Val res = SUCCESS;
	
	uint32 sn = atoi(strtok(line, ","));
	
	char id[ID_LENGTH];
	strcpy(id, strtok(line, ","));

	char id_cpy[ID_LENGTH];
	strcpy(id_cpy, id);

	uint32 sys_num = atoi(strtok(id_cpy, "_"));
	
	uint32 dir_sn = atoi(strtok(line, ","));

	uint32 block_amount = atoi(strtok(line, ","));

	PBlock_with_container bwc;
	block_with_container_pool_alloc(data_set->block_with_container_pool, block_amount, &bwc);

	uint32 block_sn, block_size;
	for (uint32 i=0;i<block_amount;i++)
	{
		block_sn = atoi(strtok(line, ","));
		block_size = atoi(strtok(line, ","));
		dedup_data_set_set_block_size(data_set, block_sn, block_size);
		bwc[i].block_sn = block_sn;
	}

	res = dedup_file_create(&(data_set->file_arr[sn]), sn,  sys_num, id, dir_sn, block_amount, bwc);

	return SUCCESS;
}
Dedup_Error_Val dedup_data_set_set_block_size(PDedup_data_set data_set, uint32 block_sn, uint32 block_size)
{
	block_set_size(&(data_set->block_arr[block_sn]), block_size);

	return SUCCESS;
}
Dedup_Error_Val dedup_data_set_add_block(PDedup_data_set data_set, char* line, uint32 length)
{
	char* letter = strtok(line, ",");
	if (!((strcmp(letter, "B") == 0 && data_set->is_block_file) || (strcmp(letter, "P") == 0 && !data_set->is_block_file)))
		return INVALID_ARGUMENT_FAILURE;

	Dedup_Error_Val res = SUCCESS;

	uint32 sn = atoi(strtok(line, ","));

	char id[ID_LENGTH];
	strcpy(id, strtok(line, ","));

	char id_cpy[ID_LENGTH];
	strcpy(id_cpy, id);

	uint32 shared_by_num_files = atoi(strtok(line, ","));

	res = block_create(&(data_set->block_arr[sn]), sn, id,shared_by_num_files);

	return res;
}
//Dedup_Error_Val dedup_data_set_analyze_to_containers(PDedup_data_set data_set);
//Dedup_Error_Val dedup_data_set_delete_system(PDedup_data_set data_set, uint32 system_sn);
//Dedup_Error_Val dedup_data_set_print_active_systems(PDedup_data_set data_set);
