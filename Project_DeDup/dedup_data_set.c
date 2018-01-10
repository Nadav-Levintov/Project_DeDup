
#include "dedup_data_set.h"

Dedup_Error_Val dedup_data_print_dfile(PDedup_data_set data_set, FILE *pFile, PDedup_File pDedup_file);
Dedup_Error_Val dedup_data_print_container(PDedup_data_set data_set, FILE *pFile, const PContainer pContainer);

Dedup_Error_Val dedup_data_set_init_args(PDedup_data_set data_set, uint32 containers_max_size, uint32 max_distance, uint32 max_pointers)
{
	memset(data_set, 0, sizeof(Dedup_data_set));

	data_set->max_distance_between_containers_for_file = max_distance;
	data_set->max_pointers_to_block = max_pointers;
	data_set->max_container_size = containers_max_size;

	return SUCCESS;
}

Dedup_Error_Val dedup_data_set_init_arrays(PDedup_data_set data_set, uint32 num_of_files, uint32 num_of_blocks, uint32 num_of_dirs)
{
	Dedup_Error_Val res = SUCCESS;

	data_set->num_of_dirs = num_of_dirs;
	// This is known in advance so we use malloc because this is only being done one time.
	data_set->num_of_blocks = num_of_blocks;
	data_set->block_arr = calloc(num_of_blocks, sizeof(Block));

	if (data_set->block_arr == NULL)
	{
		return ALLOCATION_FAILURE;
	}


	data_set->num_of_files = num_of_files;
	data_set->file_arr = calloc(num_of_files, sizeof(Dedup_File));
	if (data_set->file_arr == NULL)
	{
		free(data_set->block_arr);
		return ALLOCATION_FAILURE;
	}


	res = block_with_container_pool_init(&data_set->block_with_container_pool);
	if (res != SUCCESS)
	{
		free(data_set->block_arr);
		free(data_set->file_arr);
	}

	data_set->container_arr.length = 1;

	return res;
}
Dedup_Error_Val dedup_data_set_destroy(PDedup_data_set data_set)
{
	assert(NULL != data_set);

	/*Destroy all blocks*/
	free(data_set->block_arr);

	/*Destroy all files*/
	free(data_set->file_arr);

	/*Destroy block_with_container_pool*/
	block_with_container_pool_destroy(&(data_set->block_with_container_pool));

	/*Destroy memory pool*/
	memory_pool_destroy(&data_set->mem_pool);
	
	/* del temp file */
	assert(remove(data_set->file_name_for_dir) == 0);

	return SUCCESS;
}


Dedup_Error_Val dedup_data_set_add_file(PDedup_data_set data_set, char* line)
{
	if (strcmp(strtok(line, ","), "F") != 0)
		return INVALID_ARGUMENT_FAILURE;

	Dedup_Error_Val res = SUCCESS;

	uint32 sn = atoi(strtok(NULL, ","));

	char id[ID_LENGTH];
	strcpy(id, strtok(NULL, ","));

	char id_cpy[ID_LENGTH];
	strcpy(id_cpy, id);

	uint32 dir_sn = atoi(strtok(NULL, ","));

	uint32 block_amount = atoi(strtok(NULL, ","));

	/* The #blocks is known so we will not use dynamic array here */
	PBlock_with_container bwc_array;
	block_with_container_pool_alloc(&data_set->block_with_container_pool, block_amount * sizeof(Block_with_container), &bwc_array);

	uint32 block_sn, block_size;
	for (uint32 i = 0; i < block_amount; i++)
	{
		block_sn = atoi(strtok(NULL, ","));
		block_size = atoi(strtok(NULL, ","));
		data_set->block_arr[block_sn].size = block_size;
		bwc_array[i].block_sn = block_sn;
	}

	uint32 sys_num = atoi(strtok(id_cpy, "_"));

	res = dedup_file_create(&(data_set->file_arr[sn]), sn, sys_num, id, dir_sn, block_amount, bwc_array);
	assert(res == SUCCESS);

	return SUCCESS;
}
Dedup_Error_Val dedup_data_set_analyze_to_containers(PDedup_data_set data_set)
{
	Dedup_Error_Val ret_val = SUCCESS;
	uint32 curr_file_sn, curr_block_sn;
	uint32 *containers_filled = &(data_set->num_of_containers_filled);
	PDedup_File curr_file;
	PBlock curr_block;
	PContainer curr_container;
	PContainer_dynamic_array container_arr = &(data_set->container_arr);
	ret_val = container_dynamic_array_get(container_arr, 0, &curr_container);
	assert(ret_val == SUCCESS);
	uint32 currentSystemNum = 0;

	for (curr_file_sn = 0; curr_file_sn < data_set->num_of_files; curr_file_sn++)
	{
		curr_file = &(data_set->file_arr[curr_file_sn]);

		// check if new system and update sys_array
		if (curr_file->sys_num != currentSystemNum)
		{
			assert(curr_file->sys_num > currentSystemNum);
			data_set->system_active[curr_file->sys_num] = true;
			currentSystemNum = curr_file->sys_num;
			data_set->system_file_index[curr_file->sys_num] = curr_file_sn;
		}

		/* For each file iterate over all blocks */
		for (uint32 block_index = 0; block_index < curr_file->block_amount; block_index++)
		{
			curr_block_sn = curr_file->block_with_container_array[block_index].block_sn;
			curr_block = &(data_set->block_arr[curr_block_sn]);

			bool not_in_container = curr_block->last_container_sn == BLOCK_NOT_IN_CONTAINER;
			bool max_distance_passed = (data_set->max_distance_between_containers_for_file != 0) && ((*containers_filled) - curr_block->last_container_sn > data_set->max_distance_between_containers_for_file);
			bool max_pointers_passed = (data_set->max_pointers_to_block != 0) && (curr_block->last_container_ref_count == data_set->max_pointers_to_block);

			/* For each block check if we need to insert it to the current container or not */
			if (not_in_container || max_distance_passed || max_pointers_passed)
			{
				/* We need to insert current block to the current container */
				if (curr_container->size + curr_block->size > data_set->max_container_size)
				{
					/* Current container cannot hold the block, lets open a new container */
					//assert(data_set->max_container_size > curr_block->size);
					if (data_set->max_container_size < curr_block->size)
					{
						printf("Block with SN= %u size is larger than the Max container size!\n", curr_block->sn);
						printf("Block with SN= %u size is: %u and the max allowed size is:%u\n", curr_block->sn, curr_block->size, data_set->max_container_size);
						//TODO: create a clean exit function - mainly del the temp DIR file...
						assert(0);
					}
					ret_val = container_dynamic_array_add_and_get(container_arr, &data_set->mem_pool, &curr_container);
					assert(ret_val == SUCCESS);
					(*containers_filled)++;
				}

				ret_val = container_add_file(curr_container, &data_set->mem_pool, curr_file_sn);
				assert(ret_val == SUCCESS);

				ret_val = container_add_block(curr_container, &data_set->mem_pool, curr_block_sn, curr_block->size);
				assert(ret_val == SUCCESS);

				ret_val = block_add_container(curr_block, &data_set->mem_pool, *containers_filled);
				assert(ret_val == SUCCESS);

				/* we update the file that the current block is in the current container */
				curr_file->block_with_container_array[block_index].container_sn = *containers_filled;

			}
			else
			{
				/* Block is already in a container, lets update the container with the new file sn and the file with the container sn, also update continer ref count for block */
				PContainer temp;
				ret_val= container_dynamic_array_get(container_arr, curr_block->last_container_sn, &temp);
				assert(ret_val == SUCCESS);
				ret_val = container_add_file(temp, &data_set->mem_pool, curr_file_sn);
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
	Dedup_Error_Val ret = SUCCESS;
	if (!data_set->system_active[system_sn])
	{
		return SUCCESS;
	}

	uint32 curr_file_sn = data_set->system_file_index[system_sn], curr_block_sn = 0, curr_continer_sn = 0, curr_ref_count = 0;
	PDedup_File curr_file = &(data_set->file_arr[curr_file_sn]);
	PContainer curr_container = NULL;
	PBlock curr_block = NULL;
	PContainer_dynamic_array container_array = &(data_set->container_arr);

	/*Loop over all the files*/
	while (curr_file_sn < data_set->num_of_files && curr_file->sys_num == system_sn)
	{
		for (uint32 block_index = 0; block_index < curr_file->block_amount; block_index++)
		{
			curr_block_sn = curr_file->block_with_container_array[block_index].block_sn;
			curr_continer_sn = curr_file->block_with_container_array[block_index].container_sn;
			curr_block = &(data_set->block_arr[curr_block_sn]);
			ret=container_dynamic_array_get(container_array, curr_continer_sn, &curr_container);
			assert(ret == SUCCESS);
			ret = container_del_file(curr_container, curr_file_sn);
			assert(ret == SUCCESS);
			ret = block_container_decrece_ref_count(curr_block, curr_continer_sn, &curr_ref_count);
			assert(ret == SUCCESS);
			if (curr_ref_count == 0)
			{
				ret = container_del_block(curr_container, curr_block_sn, curr_block->size);
				assert(ret == SUCCESS);
			}
		}

		curr_file_sn++;
		curr_file = &(data_set->file_arr[curr_file_sn]);
	}

	data_set->system_active[system_sn] = false;

	return SUCCESS;
}

Dedup_Error_Val dedup_data_set_add_block(PDedup_data_set data_set, char* line)
{
	char* letter = strtok(line, ",");
	if (!((strcmp(letter, "B") == 0 && data_set->is_block_file) || (strcmp(letter, "P") == 0 && !data_set->is_block_file)))
		return INVALID_ARGUMENT_FAILURE;

	Dedup_Error_Val res = SUCCESS;

	uint32 sn = atoi(strtok(NULL, ","));

	char id[ID_LENGTH];
	strcpy(id, strtok(NULL, ","));

	char id_cpy[ID_LENGTH];
	strcpy(id_cpy, id);

	uint32 shared_by_num_files = atoi(strtok(NULL, ","));

	res = block_init(&(data_set->block_arr[sn]), sn, id, shared_by_num_files);

	return res;
}

Dedup_Error_Val dedup_data_set_print_active_systems(PDedup_data_set data_set, char *file_name)
{

	Dedup_Error_Val res = SUCCESS;
	FILE *pFile;

	pFile = fopen(file_name, "w");
	assert(pFile != NULL);

	/*write files*/
	int system_num;
	for (system_num = 1; system_num < MAX_SYSTEMS; system_num++)
	{
		uint32 file_sn = 0;
		PDedup_File currentFile = NULL;

		/*If system is not active check the next one*/
		if (data_set->system_active[system_num] == false)
		{
			continue;
		}
		file_sn = data_set->system_file_index[system_num];
		currentFile = &(data_set->file_arr[file_sn]);

		/*Go over all the files belong to this system*/
		while (currentFile->sys_num == system_num)
		{
			currentFile = &data_set->file_arr[file_sn];

			res = dedup_data_print_dfile(data_set, pFile, currentFile);
			assert(res == SUCCESS);

			file_sn++;
		}

	}

	/*write containers*/
	uint32 container_sn;
	PContainer pCurrentContainer = NULL;

	for (container_sn = 0; container_sn < data_set->num_of_containers_filled; container_sn++)
	{
		res = container_dynamic_array_get(&(data_set->container_arr), container_sn, &pCurrentContainer);
		assert(res == SUCCESS);
		res = dedup_data_print_container(data_set, pFile, pCurrentContainer);
		assert(res == SUCCESS);
	}

	/*write directories*/
	FILE *pTempFile = NULL;
	char line[LINE_LENGTH];
	char line_temp[LINE_LENGTH];
	pTempFile = fopen(data_set->file_name_for_dir, "r");
	assert(pTempFile != NULL);
	char* prefix = NULL;
	uint32 systen_sn = 0;


	while (fgets(line, sizeof(line), pTempFile)) 
	{
		/*	TODO: what if line is larger than LINE_LENGTH? we may lose data here because lines in file may be larger than 1000
			then the fgets takes the next 1000 chars (until \n) and the first char is not the active system so we will
			not print the line!
		*/
		/*Check if this directory is from active system*/
		strcpy(line_temp, line);
		prefix = strtok(line_temp, ",");
		prefix = strtok(NULL, ",");
		prefix = strtok(NULL, ",");
		systen_sn = atoi(strtok(prefix, "_"));
		if (data_set->system_active[systen_sn])
		{
			fputs(line, pFile);
		}
	}

	/*TODO: when destroying some lines are added to the file, need to check why this happens and not when we close the temp. */

	/*close file*/
	fclose(pTempFile);

	return SUCCESS;
}

Dedup_Error_Val dedup_data_print_dfile(PDedup_data_set data_set, FILE *pFile, PDedup_File pDedup_file)
{
	Dedup_Error_Val res = SUCCESS;
	char buffer[LINE_LENGTH] = { 0 };
	char tmpArray[LINE_LENGTH] = { 0 };
	uint32 *container_sns = calloc((pDedup_file->block_amount) , sizeof(uint32));
	uint32 containersIndx = 0;
	strcat(buffer, "F,");
	sprintf(tmpArray, "%u", pDedup_file->sn);
	strcat(buffer, tmpArray);

	strcat(buffer, ",");
	strcat(buffer, pDedup_file->id);

	strcat(buffer, ",");
	sprintf(tmpArray, "%u", pDedup_file->sn);
	strcat(buffer, tmpArray);


	uint32 containerNumInArray;
	uint32 containerSn;
	PContainer pContainer = NULL;

	for (containerNumInArray = 0; containerNumInArray < pDedup_file->block_amount; containerNumInArray++)
	{

		containerSn = pDedup_file->block_with_container_array[containerNumInArray].container_sn;
		res = container_dynamic_array_get(&data_set->container_arr, containerSn, &pContainer);
		assert(res == SUCCESS);

		bool new_contaier = true;
		uint32 i;
		for (i = 0; i < containersIndx; i++)
		{
			if (containerSn == container_sns[i])
			{
				new_contaier = false;
			}
		}
		if (new_contaier)
		{
			/*Add containrSn to containers*/
			container_sns[containersIndx] = containerSn;
			containersIndx++;

			strcat(buffer, ",");
			sprintf(tmpArray, "%u", containerSn);
			strcat(buffer, tmpArray);

			strcat(buffer, ",");
			sprintf(tmpArray, "%u", pContainer->size);
			strcat(buffer, tmpArray);
		}
	}
	strcat(buffer, "\n");
	fputs(buffer, pFile);
	free(container_sns);
	return res;
}

Dedup_Error_Val dedup_data_print_container(PDedup_data_set data_set, FILE *pFile, const PContainer pContainer)
{
	char buffer[LINE_LENGTH] = { 0 };
	char tmpArray[LINE_LENGTH] = { 0 };
	assert(pContainer);

	/* Write first line of the container*/
	strcat(buffer, "C,");
	sprintf(tmpArray, "%u", pContainer->sn);
	strcat(buffer, tmpArray);

	strcat(buffer, ",");
	sprintf(tmpArray, "%u", pContainer->size);
	strcat(buffer, tmpArray);

	strcat(buffer, ",");
	sprintf(tmpArray, "%u", pContainer->num_of_files_using);
	strcat(buffer, tmpArray);

	uint32 index=0, value, printed_files = 0;
	while (printed_files < pContainer->num_of_files_using)
	{
		assert(DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR != dynamic_array_get(&pContainer->file_array, index, &value));
		if (value != REMOVED_SN)
		{
			if (strlen(buffer) + strlen(tmpArray) + 1 >= LINE_LENGTH)
			{
				fputs(buffer, pFile);
				memset(buffer, 0, LINE_LENGTH);
			}
			strcat(buffer, ",");
			sprintf(tmpArray, "%u", value);
			strcat(buffer, tmpArray);
			printed_files++;
		}
		index++;
	}
	strcat(buffer, "\n");
	fputs(buffer, pFile);

	/* Write second line of the container*/

	memset(buffer, 0, LINE_LENGTH);

	strcat(buffer, "M,");
	sprintf(tmpArray, "%u", pContainer->sn);
	strcat(buffer, tmpArray);

	strcat(buffer, ",");
	sprintf(tmpArray, "%u", pContainer->size);
	strcat(buffer, tmpArray);

	strcat(buffer, ",");
	sprintf(tmpArray, "%u", pContainer->num_of_blocks);
	strcat(buffer, tmpArray);
	index = 0;
	uint32 num_of_blocks_printed = 0;
	while (num_of_blocks_printed < pContainer->num_of_blocks)
	{
		assert(DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR != dynamic_array_get(&pContainer->block_array, index, &value));
		if (value != REMOVED_SN)
		{
			if (strlen(buffer) + strlen(tmpArray) + 1 >= LINE_LENGTH)
			{
				fputs(buffer, pFile);
				memset(buffer, 0, LINE_LENGTH);
			}
			strcat(buffer, ",");
			sprintf(tmpArray, "%u", value);
			strcat(buffer, tmpArray);
			num_of_blocks_printed++;
		}
		index++;
	}
	strcat(buffer, "\n");
	fputs(buffer, pFile);
	return SUCCESS;
}
