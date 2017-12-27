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
	memset(data_set, 0, sizeof(Dedup_data_set));

	// This is known in advance so we use malloc because this is only being done one time.
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

	/* The #blocks is known so we will not use dynamic array here */
	PBlock_with_container bwc;
	block_with_container_pool_alloc(data_set->block_with_container_pool, block_amount, &bwc);

	uint32 block_sn, block_size;
	for (uint32 i = 0; i < block_amount; i++)
	{
		block_sn = atoi(strtok(line, ","));
		block_size = atoi(strtok(line, ","));
		data_set->block_arr[block_sn].size = block_size;
		bwc[i].block_sn = block_sn;
	}

	res = dedup_file_create(&(data_set->file_arr[sn]), sn, sys_num, id, dir_sn, block_amount, bwc);
	assert(res == SUCCESS);

	return SUCCESS;
}
Dedup_Error_Val dedup_data_set_analyze_to_containers(PDedup_data_set data_set)
{
	Dedup_Error_Val ret_val = SUCCESS;
	uint32 curr_file_sn, curr_block_sn, *containers_filled = &(data_set->num_of_containers_filled);
	PDedup_File curr_file;
	PBlock curr_block;
	PContainer curr_container;
	PContainer_dynamic_array container_arr = &(data_set->container_arr);
	ret_val = container_dynamic_array_get(container_arr, 0, &curr_container);
	assert(ret_val == SUCCESS);

	for (curr_file_sn = 0; curr_file_sn < data_set->num_of_files; curr_file_sn++)
	{
		curr_file = &(data_set->file_arr[curr_file_sn]);
		/* For each file iterate over all blocks */
		for (uint32 block_index = 0; block_index < curr_file->block_amount; block_index++)
		{
			curr_block_sn = curr_file->block_with_container_array[block_index].block_sn;
			curr_block = &(data_set->block_arr[curr_block_sn]);
			bool not_in_container = curr_block->last_container_sn == BLOCK_NOT_IN_CONTAINER;
			bool max_distance_passed = (*containers_filled) - curr_block->last_container_sn > data_set->max_distance_between_containers_for_file;
			bool max_pointers_passed = curr_block->last_container_sn == data_set->max_pointers_to_block;

			/* For each block check if we need to insert it to the current container or not */
			if (not_in_container || max_distance_passed || max_pointers_passed)
			{
				/* We need to insert current block to the current container */
				if (curr_container->size + curr_block->size > data_set->max_container_size)
				{
					/* Current container cannot hold the block, lets open a new container */
					assert(data_set->max_container_size > curr_block->size);
					ret_val = container_dynamic_array_add_and_get(container_arr, data_set->mem_pool, &curr_container);
					assert(ret_val == SUCCESS);
					*containers_filled++;
				}

				ret_val = container_add_file(curr_container, data_set->mem_pool, curr_file_sn);
				assert(ret_val == SUCCESS);

				ret_val = container_add_block(curr_container, data_set->mem_pool, curr_block_sn, curr_block->size);
				assert(ret_val == SUCCESS);

				ret_val = block_add_container(curr_block, data_set->mem_pool, *containers_filled);
				assert(ret_val == SUCCESS);

				/* we update the file that the current block is in the current container */
				curr_file->block_with_container_array[block_index].container_sn = *containers_filled;

			}
			else
			{
				/* Block is already in a container, lets update the container with the new file sn and the file with the container sn */
				ret_val = container_add_file(curr_container, data_set->mem_pool, curr_file_sn);
				assert(ret_val == SUCCESS);
				ret_val = block_advance_last_container_ref_count(curr_block);
				assert(ret_val == SUCCESS);
				ret_val = block_advance_last_container_ref_count(curr_block);
				assert(ret_val == SUCCESS);

				/* we update the file that the current block is in the current container */
				curr_file->block_with_container_array[block_index].container_sn = curr_block->last_container_sn;
			}
		}
	}

	return SUCCESS;
}
Dedup_Error_Val dedup_data_set_delete_system(PDedup_data_set data_set, uint32 system_sn)
{
	return SUCCESS;
}
Dedup_Error_Val dedup_data_set_print_active_systems(PDedup_data_set data_set)
{
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

	res = block_init(&(data_set->block_arr[sn]), sn, id, shared_by_num_files);

	return res;
}

