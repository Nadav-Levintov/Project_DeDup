#include <time.h>
#include "dedup_data_set.h"

char data_set_line1[LINE_LENGTH] = { 0 };
char data_set_line2[LINE_LENGTH] = { 0 };
char data_set_line3[LINE_LENGTH] = { 0 };

Dedup_Error_Val dedup_data_set_init_args(PDedup_data_set data_set, char* file_name, uint32 containers_max_size, uint32 max_distance, uint32 max_pointers)
{
	char error_file_name[MAX_FILE_NAME];
	char time_string[MAX_FILE_NAME];
	FILE* error_file = NULL;
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	memset(data_set, 0, sizeof(Dedup_data_set));

	data_set->max_distance_between_containers_for_file = max_distance;
	data_set->max_pointers_to_block = max_pointers;
	data_set->max_container_size = containers_max_size;
	data_set->num_of_active_systems = 0;
	data_set->num_of_systems = 0;
	
	strcpy(error_file_name, file_name);
	strcpy(data_set->error_file_name, strtok(error_file_name, "."));
	sprintf(time_string, "_%d_%02d_%d_%02d_%02d_%02d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, 
		timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	strcat(data_set->error_file_name, time_string);
	strcat(data_set->error_file_name, "_error.txt");
	error_file = fopen(data_set->error_file_name, "w");

	assert(error_file != NULL);
	fclose(error_file);

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
	data_set->file_arr = calloc(num_of_files+1, sizeof(Dedup_File));
	data_set->file_arr[num_of_files].sys_num = 0;
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


Dedup_Error_Val dedup_data_set_add_file(PDedup_data_set data_set, char* line, FILE* fptr)
{
	bool line_end_with_comma = false;
	if (line[strlen(line) - 1] == ',')
		line_end_with_comma = true;
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
	char* line_ptr;
	if (block_amount > 0) {

		block_with_container_pool_alloc(&data_set->block_with_container_pool, block_amount * sizeof(Block_with_container), &bwc_array);

		uint32 block_sn, block_size;
		for (uint32 i = 0; i < block_amount; i++)
		{
			line_ptr = strtok(NULL, ",");
			if (!line_ptr)
			{
				/* There is a line over flow, we need to fix last block size */
				bool last_line_ended_with_comma = line_end_with_comma;
				line_ptr = fgets(line, LINE_LENGTH, fptr);
				line_end_with_comma = false;
				if (line[strlen(line) - 1] == ',')
				{
					line_end_with_comma = true;
				}

				if (line[0] == ',')
				{
					last_line_ended_with_comma = true;
				}

				line_ptr = strtok(line, ",");

				if (!last_line_ended_with_comma)
				{
					data_set->block_arr[block_sn].size *= pow_aux(10, strlen(line_ptr));
					data_set->block_arr[block_sn].size += atoi(line_ptr);
					line_ptr = strtok(NULL, ",");
				}
			}
			block_sn = atoi(line_ptr);
			line_ptr = strtok(NULL, ",");
			if (!line_ptr)
			{
				/* There is a line over flow */
				bool last_line_ended_with_comma = line_end_with_comma;
				line_ptr = fgets(line, LINE_LENGTH, fptr);
				line_end_with_comma = false;

				if (line[strlen(line) - 1] == ',')
				{
					line_end_with_comma = true;
				}

				if (line[0] == ',')
				{
					last_line_ended_with_comma = true;
				}

				line_ptr = strtok(line, ",");
				if (!last_line_ended_with_comma) {
					/* we need to fix block sn */
					block_sn *= pow_aux(10, strlen(line_ptr));
					block_sn += atoi(line_ptr);
					line_ptr = strtok(NULL, ",");
				}
			}
			block_size = atoi(line_ptr);
			data_set->block_arr[block_sn].size = block_size;
			bwc_array[i].block_sn = block_sn;
		}
	}
	else
	{
		res = dedup_file_create(&(data_set->file_arr[sn]), REMOVED_SN, 0, id, dir_sn, 0, NULL);
		assert(res == SUCCESS);
	}
	uint32 sys_num = atoi(strtok(id_cpy, "_"));
	if (block_amount > data_set->max_num_of_containers)
	{
		data_set->max_num_of_containers = block_amount;
	}

	res = dedup_file_create(&(data_set->file_arr[sn]), sn, sys_num, id, dir_sn, block_amount, bwc_array);
	assert(res == SUCCESS);

	return SUCCESS;
}
Dedup_Error_Val dedup_data_set_analyze_to_containers(PDedup_data_set data_set)
{
	Dedup_Error_Val ret_val = SUCCESS;
	uint32 curr_file_sn, curr_block_sn;
	uint32 *containers_filled = &(data_set->num_of_containers_filled);
	uint64 checkMaxSize =0;
	PDedup_File curr_file;
	PBlock curr_block;
	PContainer curr_container;
	PContainer_dynamic_array container_arr = &(data_set->container_arr);
	ret_val = container_dynamic_array_get(container_arr, 0, &curr_container);
	assert(ret_val == SUCCESS);
	uint32 currentSystemNum = 0;
	uint32 blockSize = 0;

	for (curr_file_sn = 0; curr_file_sn < data_set->num_of_files; curr_file_sn++)
	{
		curr_file = &(data_set->file_arr[curr_file_sn]);
		if (curr_file->sn == REMOVED_SN)
		{
			continue;
		}

		// check if new system and update sys_array
		if (curr_file->sys_num != currentSystemNum)
		{
			assert(curr_file->sys_num > currentSystemNum);
			data_set->system_active[curr_file->sys_num] = true;

			data_set->num_of_systems++;
			data_set->num_of_active_systems++;

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
			blockSize = curr_block->size;
			if (not_in_container || max_distance_passed || max_pointers_passed)
			{
				/* We need to insert current block to the current container */
				checkMaxSize = (uint64)curr_container->size + (uint64)curr_block->size;
				if (checkMaxSize > data_set->max_container_size)
				{
					/* Current container cannot hold the block, lets open a new container */
					if (data_set->max_container_size < curr_block->size)
					{
						FILE* error_file = NULL;
						error_file = fopen(data_set->error_file_name, "a");
						assert(error_file != NULL);
						
						memset(data_set_line1, 0, LINE_LENGTH);
						memset(data_set_line2, 0, LINE_LENGTH);
						strcat(data_set_line1, "Block with SN: ");
						sprintf(data_set_line2, "%u ", curr_block->sn);
						strcat(data_set_line1, data_set_line2);

						strcat(data_set_line1, "size is: ");
						sprintf(data_set_line2, "%u ", curr_block->size);
						strcat(data_set_line1, data_set_line2);

						strcat(data_set_line1, "larger than the Max container size allowed:  ");
						sprintf(data_set_line2, "%u ", data_set->max_container_size);
						strcat(data_set_line1, data_set_line2);

						strcat(data_set_line1, "\n");
						fputs(data_set_line1, error_file);
						blockSize = data_set->max_container_size;
						fclose(error_file);
					}
					ret_val = container_dynamic_array_add_and_get(container_arr, &data_set->mem_pool, &curr_container);
					assert(ret_val == SUCCESS);
					(*containers_filled)++;

				}

				ret_val = container_add_file(curr_container, &data_set->mem_pool, curr_file_sn);
				assert(ret_val == SUCCESS);

				ret_val = container_add_block(curr_container, &data_set->mem_pool, curr_block_sn, blockSize);
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
				ret_val = container_dynamic_array_get(container_arr, curr_block->last_container_sn, &temp);
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
	if (system_sn > data_set->num_of_systems || !data_set->system_active[system_sn])
	{
		printf("System %d already deleted or is not in the input file.\n", system_sn);
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
			ret = container_dynamic_array_get(container_array, curr_continer_sn, &curr_container);
			assert(ret == SUCCESS);
			ret = container_del_file(curr_container, curr_file_sn);
			assert(ret == SUCCESS);
			ret = block_container_decrece_ref_count(curr_block, curr_continer_sn, &curr_ref_count);
			assert(ret == SUCCESS && curr_ref_count != INDEX_NOT_FOUND);
			if (curr_ref_count == 0)
			{
				if(curr_block->size > data_set->max_container_size)
				{
					ret = container_del_block(curr_container, curr_block_sn, data_set->max_container_size);
				}
				else
				{
					ret = container_del_block(curr_container, curr_block_sn, curr_block->size);
				}
				assert(ret == SUCCESS);
			}
		}

		curr_file_sn++;
		curr_file = &(data_set->file_arr[curr_file_sn]);
	}

	data_set->system_active[system_sn] = false;
	data_set->system_deletion_order[data_set->num_of_systems - data_set->num_of_active_systems] = system_sn;
	data_set->num_of_active_systems--;

	return SUCCESS;
}

Dedup_Error_Val dedup_data_set_add_block(PDedup_data_set data_set, char* line, FILE* fptr)
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
	uint32 *container_sns = calloc(data_set->max_num_of_containers, sizeof(uint32));
	assert(container_sns != NULL);
	pFile = fopen(file_name, "w");
	assert(pFile != NULL);

	/*print header*/
	res = dedup_data_print_header(data_set, pFile);

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
		while (data_set->file_arr[file_sn].sys_num == system_num)
		{
			currentFile = &data_set->file_arr[file_sn];
			res = dedup_data_print_dfile(data_set, pFile, currentFile, container_sns);
			assert(res == SUCCESS);
			file_sn++;
		}
	}

	/*write containers*/
	uint32 container_sn;
	PContainer pCurrentContainer = NULL;

	for (container_sn = 0; container_sn <= data_set->num_of_containers_filled; container_sn++)
	{
		res = container_dynamic_array_get(&(data_set->container_arr), container_sn, &pCurrentContainer);
		assert(res == SUCCESS);
		res = dedup_data_print_container(data_set, pFile, pCurrentContainer);
		assert(res == SUCCESS);
	}

	/*write directories*/
	FILE *pTempFile = NULL;
	pTempFile = fopen(data_set->file_name_for_dir, "r");
	assert(pTempFile != NULL);
	char* prefix = NULL;
	uint32 systen_sn = 0;
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);
	memset(data_set_line3, 0, LINE_LENGTH);

	char* line_ptr = fgets(data_set_line1, LINE_LENGTH, pTempFile);
	while (line_ptr)
	{
		/*Check if this directory is from active system*/
		strcpy(data_set_line2, data_set_line1);
		prefix = strtok(data_set_line2, ",");
		prefix = strtok(NULL, ",");
		prefix = strtok(NULL, ",");
		systen_sn = atoi(strtok(prefix, "_"));
		if (data_set->system_active[systen_sn])
		{
			fputs(data_set_line1, pFile);
		}
		size_t curr_line_len = strlen(data_set_line1) - 1;
		while (data_set_line1[curr_line_len] != '\n')
		{
			line_ptr = fgets(data_set_line1, LINE_LENGTH, pTempFile);
			if (data_set->system_active[systen_sn])
			{
				fputs(data_set_line1, pFile);
			}
			curr_line_len = strlen(data_set_line1) - 1;
		}
		line_ptr = fgets(data_set_line1, LINE_LENGTH, pTempFile);
	}

	/*close file*/
	fclose(pTempFile);
	free(container_sns);
	return SUCCESS;
}


Dedup_Error_Val dedup_data_print_header(PDedup_data_set data_set, FILE *pFile)
{
	Dedup_Error_Val res = SUCCESS;
	uint32 i = 0;

	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Container max size: ");
	sprintf(data_set_line2, "%u", data_set->max_container_size);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Max distance between containers: ");
	sprintf(data_set_line2, "%u", data_set->max_distance_between_containers_for_file);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Max pointers to block: ");
	sprintf(data_set_line2, "%u", data_set->max_pointers_to_block);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Num of files: ");
	sprintf(data_set_line2, "%u", data_set->num_of_files);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Num of directories: ");
	sprintf(data_set_line2, "%u", data_set->num_of_dirs);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Num of containers: ");
	sprintf(data_set_line2, "%u", data_set->num_of_containers_filled+1);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Num of blocks: ");
	sprintf(data_set_line2, "%u", data_set->num_of_blocks);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Num of total systems: ");
	sprintf(data_set_line2, "%u", data_set->num_of_systems);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Num of active systems: ");
	sprintf(data_set_line2, "%u", data_set->num_of_active_systems);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);

	strcat(data_set_line1, "# Deletion order: ");
	for(i = 0; i < data_set->num_of_systems - data_set->num_of_active_systems; i++)
	{
			sprintf(data_set_line2, "%u ", data_set->system_deletion_order[i]);
			strcat(data_set_line1, data_set_line2);	
	}
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);
	return res;
}

Dedup_Error_Val dedup_data_print_dfile(PDedup_data_set data_set, FILE *pFile, PDedup_File pDedup_file, uint32 * container_sns)
{
	Dedup_Error_Val res = SUCCESS;
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);
	memset(data_set_line3, 0, LINE_LENGTH);

	uint32 containersIndx = 0;
	strcat(data_set_line1, "F,");
	sprintf(data_set_line2, "%u", pDedup_file->sn);
	strcat(data_set_line1, data_set_line2);

	strcat(data_set_line1, ",");
	strcat(data_set_line1, pDedup_file->id);

	strcat(data_set_line1, ",");
	sprintf(data_set_line2, "%u", pDedup_file->dir_sn);

	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, ",");

	uint32 containerNumInArray;
	uint32 containerSn;
	uint32 containersCount=0;
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
			containersCount++;

			sprintf(data_set_line2, "%u", containerSn);
			strcat(data_set_line3, ",");
			strcat(data_set_line3, data_set_line2);

			sprintf(data_set_line2, "%u", pContainer->size);
			strcat(data_set_line3, ",");
			strcat(data_set_line3, data_set_line2);
		}
	}
	sprintf(data_set_line2, "%u", containersCount);
	strcat(data_set_line1, data_set_line2);

	strcat(data_set_line1, data_set_line3);

	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	return res;
}

Dedup_Error_Val dedup_data_print_container(PDedup_data_set data_set, FILE *pFile, const PContainer pContainer)
{
	memset(data_set_line1, 0, LINE_LENGTH);
	memset(data_set_line2, 0, LINE_LENGTH);
	assert(pContainer);

	/* Write first line of the container*/
	strcat(data_set_line1, "C,");
	sprintf(data_set_line2, "%u", pContainer->sn);
	strcat(data_set_line1, data_set_line2);

	strcat(data_set_line1, ",");
	sprintf(data_set_line2, "%u", pContainer->size);
	strcat(data_set_line1, data_set_line2);

	strcat(data_set_line1, ",");
	sprintf(data_set_line2, "%u", pContainer->num_of_files_using);
	strcat(data_set_line1, data_set_line2);

	uint32 index = 0, value, printed_files = 0;
	while (printed_files < pContainer->num_of_files_using)
	{
		assert(DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR != dynamic_array_get(&pContainer->file_array, index, &value));
		if (value != REMOVED_SN)
		{
			if (strlen(data_set_line1) + strlen(data_set_line2) + 1 >= LINE_LENGTH)
			{
				fputs(data_set_line1, pFile);
				memset(data_set_line1, 0, LINE_LENGTH);
			}
			strcat(data_set_line1, ",");
			sprintf(data_set_line2, "%u", value);
			strcat(data_set_line1, data_set_line2);
			printed_files++;
		}
		index++;
	}
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);

	/* Write second line of the container*/

	memset(data_set_line1, 0, LINE_LENGTH);

	strcat(data_set_line1, "M,");
	sprintf(data_set_line2, "%u", pContainer->sn);
	strcat(data_set_line1, data_set_line2);

	strcat(data_set_line1, ",");
	sprintf(data_set_line2, "%u", pContainer->size);
	strcat(data_set_line1, data_set_line2);

	strcat(data_set_line1, ",");
	sprintf(data_set_line2, "%u", pContainer->num_of_blocks);
	strcat(data_set_line1, data_set_line2);
	index = 0;
	uint32 num_of_blocks_printed = 0;
	while (num_of_blocks_printed < pContainer->num_of_blocks)
	{
		assert(DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR != dynamic_array_get(&pContainer->block_array, index, &value));
		if (value != REMOVED_SN)
		{
			if (strlen(data_set_line1) + strlen(data_set_line2) + 1 >= LINE_LENGTH)
			{
				fputs(data_set_line1, pFile);
				memset(data_set_line1, 0, LINE_LENGTH);
			}
			strcat(data_set_line1, ",");
			sprintf(data_set_line2, "%u", value);
			strcat(data_set_line1, data_set_line2);
			num_of_blocks_printed++;
		}
		index++;
	}
	strcat(data_set_line1, "\n");
	fputs(data_set_line1, pFile);
	return SUCCESS;
}
