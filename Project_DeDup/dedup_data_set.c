
#include "dedup_data_set.h"

Dedup_Error_Val dedup_data_print_dfile(PDedup_data_set data_set, FILE *pFile, PDedup_File pDedup_file);
Dedup_Error_Val dedup_data_print_container(PDedup_data_set data_set, FILE *pFile, PContainer pContainer);


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
	PBlock_with_container bwc;
	block_with_container_pool_alloc(data_set->block_with_container_pool, block_amount*sizeof(Block_with_container), &bwc);

	uint32 block_sn, block_size;
	for (uint32 i = 0; i < block_amount; i++)
	{
		block_sn = atoi(strtok(NULL, ","));
		block_size = atoi(strtok(NULL, ","));
		data_set->block_arr[block_sn].size = block_size;
		bwc[i].block_sn = block_sn;
	}

	uint32 sys_num = atoi(strtok(id_cpy, "_"));

	res = dedup_file_create(&(data_set->file_arr[sn]), sn, sys_num, id, dir_sn, block_amount, bwc);
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
			assert(curr_file->sys_num < currentSystemNum);
			data_set->system_active[currentSystemNum] = true;
			currentSystemNum = curr_file->sys_num;
			data_set->system_file_index[currentSystemNum] = curr_file_sn;
		}

		/* For each file iterate over all blocks */
		for (uint32 block_index = 0; block_index < curr_file->block_amount; block_index++)
		{
			curr_block_sn = curr_file->block_with_container_array[block_index].block_sn;
			curr_block = &(data_set->block_arr[curr_block_sn]);

			bool not_in_container = curr_block->last_container_sn == BLOCK_NOT_IN_CONTAINER;
			bool max_distance_passed = (*containers_filled) - curr_block->last_container_sn > data_set->max_distance_between_containers_for_file;
			bool max_pointers_passed = curr_block->last_container_ref_count == data_set->max_pointers_to_block;

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
					(*containers_filled)++;
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
				/* Block is already in a container, lets update the container with the new file sn and the file with the container sn, also update continer ref count for block */
				ret_val = container_add_file(curr_container, data_set->mem_pool, curr_file_sn);
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
	PDynamic_array container_array = (PDynamic_array)&(data_set->container_arr);

	/*Loop over all the files*/
	while (curr_file_sn < data_set->num_of_files && curr_file->sys_num == system_sn)
	{
		for (uint32 block_index = 0; block_index < curr_file->block_amount; block_index++)
		{
			curr_block_sn = curr_file->block_with_container_array[block_index].block_sn;
			curr_continer_sn = curr_file->block_with_container_array[block_index].container_sn;
			curr_block = &(data_set->block_arr[curr_block_sn]);
			dynamic_array_get(container_array, curr_continer_sn, (uint32*)&curr_container);

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

Dedup_Error_Val dedup_data_set_print_active_systems(PDedup_data_set data_set, FILE *pFile)
{
	Dedup_Error_Val res = SUCCESS;

	/*write files*/
	int system_num;
	for(system_num = 0;  system_num < MAX_SYSTEMS; system_num++)
	{
		uint32 fileIndx = 0;
		PDedup_File currentFile = NULL;

		/*If system is not active check the next one*/
		if(data_set->system_active[system_num] == false)
		{
			continue;
		}
		fileIndx = data_set->system_file_index[system_num];

		/*Go over all the files belong to this system*/
		while(currentFile->sys_num == system_num)
		{
			currentFile = &data_set->file_arr[fileIndx];

			res = dedup_data_print_dfile(data_set, pFile, currentFile);
			assert(res == SUCCESS);

			fileIndx++;
		}

	}

	/*write containers*/
	uint32 container_sn;
	PContainer pCurrentContainer = NULL;

	for(container_sn = 0; data_set->num_of_containers_filled; container_sn++)
	{
		res = container_dynamic_array_get(&(data_set->container_arr), container_sn, (void*)pCurrentContainer);
		assert(res == SUCCESS);
		res = dedup_data_print_container(data_set, pFile, pCurrentContainer);
		assert(res == SUCCESS);
	}

	/*write directories*/
	FILE *pTempFile = NULL;
	char line[LENGTH_LENGHT];
	pTempFile = fopen(data_set->file_name_for_dir, "r");
	assert(pTempFile != NULL);
	char* prefix = NULL;
	uint32 systen_sn = 0;


	while (fgets(line, sizeof(line), pTempFile))
	{
		/*Check if this directory is from active system*/
		prefix = strtok(line, ",");
		prefix = strtok(NULL, ",");
		prefix =  strtok(NULL, ",");
		systen_sn = atoi(strtok(prefix, "_"));
		if(data_set->system_active[systen_sn])
		{
			fprintf(pFile, "%s", line);
		}
	}

	/*close file*/
	fclose(pTempFile);

	return SUCCESS;
}

Dedup_Error_Val dedup_data_set_destroy(PDedup_data_set data_set)
{
	return SUCCESS;
}

Dedup_Error_Val dedup_data_print_dfile(PDedup_data_set data_set, FILE *pFile, PDedup_File pDedup_file)
{
	Dedup_Error_Val res = SUCCESS;
	char buffer[LENGTH_LENGHT];
	char tmpArray[LENGTH_LENGHT];
	uint32 containers[pDedup_file->block_amount];
	uint32 containersIndx = 0;
	strcat(buffer, "F,");
	sprintf(buffer, "%d", pDedup_file->sn);

	strcat(buffer, ",");
	strcat(buffer, pDedup_file->id);

	strcat(buffer, ",");
	sprintf(tmpArray, "%d", pDedup_file->sn);
	strcat(buffer, tmpArray);


	int containerNumInArray;
	int containerSn;
	PContainer pContainer = NULL;

	for (containerNumInArray = 0; containerNumInArray < pDedup_file->block_amount; containerNumInArray++)
	{

		containerSn = pDedup_file->block_with_container_array[containerNumInArray].block_sn;
		res = container_dynamic_array_get(&data_set->container_arr, containerSn, &pContainer);
		assert(res == SUCCESS);

		bool new_contaier = true;
		int i;
		for(i=0; i < containersIndx; i++)
		{
			if(containerSn == containers[i])
			{
				new_contaier = false;
			}
		}
		if(new_contaier)
		{
			/*Add containrSn to containers*/
			containers[containersIndx] = containerSn;
			containersIndx ++ ;

			strcat(buffer, ",");
			sprintf(tmpArray, "%d", containerSn);
			strcat(buffer, tmpArray);

			strcat(buffer, ",");
			sprintf(tmpArray, "%d", pContainer->size);
			strcat(buffer, tmpArray);
		}
	}

	fprintf(pFile, "%s", buffer);
	return res;
}

Dedup_Error_Val dedup_data_print_container(PDedup_data_set data_set, FILE *pFile, PContainer pContainer)
{
	Dedup_Error_Val res = SUCCESS;
	char buffer[LENGTH_LENGHT];
	char tmpArray[LENGTH_LENGHT];

	/* Write first line of the container*/
	strcat(buffer, "C,");
	sprintf(buffer, "%d", pContainer->sn);

	strcat(buffer, ",");
	sprintf(tmpArray, "%d", pContainer->size);
	strcat(buffer, tmpArray);

	strcat(buffer, ",");
	sprintf(tmpArray, "%d", pContainer->num_of_files_using);
	strcat(buffer, tmpArray);

	int index;
	uint32 value;
	PContainer container = NULL;

	for (index = 0; index < pContainer->num_of_files_using; index++)
	{
		while(DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR !=
				dynamic_array_get(&pContainer->file_array, index, &value))
		{
			if(value != REMOVED_SN)
			{
				strcat(buffer, ",");
				sprintf(tmpArray, "%d", value);
				strcat(buffer, tmpArray);

				res = container_dynamic_array_get(&data_set->container_arr, index, &container);

				strcat(buffer, ",");
				sprintf(tmpArray, "%d", container->size);
				strcat(buffer, tmpArray);
			}
		}
	}

	fprintf(pFile, "%s", buffer);

	/* Write second line of the container*/

	memset(buffer, 0, LENGTH_LENGHT);

	strcat(buffer, "M,");
	sprintf(buffer, "%d", pContainer->sn);

	strcat(buffer, ",");
	sprintf(tmpArray, "%d", pContainer->size);
	strcat(buffer, tmpArray);

	strcat(buffer, ",");
	sprintf(tmpArray, "%d", pContainer->num_of_blocks);
	strcat(buffer, tmpArray);

	for (index = 0; index < pContainer->num_of_blocks; index++)
	{
		while(DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR !=
				dynamic_array_get(&pContainer->block_array, index, &value))
		{
			if(value != REMOVED_SN)
			{
				strcat(buffer, ",");
				sprintf(tmpArray, "%d", value);
				strcat(buffer, tmpArray);

				strcat(buffer, ",");
				sprintf(tmpArray, "%d", data_set->block_arr[value].size);
				strcat(buffer, tmpArray);
			}
		}
	}
	fprintf(pFile, "%s", buffer);
	return res;
}
