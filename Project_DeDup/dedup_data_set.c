#include <time.h>
#include "dedup_data_set.h"

/* buffers to be used for parsing */
char data_set_line1[LINE_LENGTH] = { 0 };
char data_set_line2[LINE_LENGTH] = { 0 };
char data_set_line3[LINE_LENGTH] = { 0 };

static void print_sns_from_tree_to_file(void* data, FILE *pFile)
{
	uint32 value = *(uint32*)(data);
	if (strlen(data_set_line1) + strlen(data_set_line2) + LINE_OVER_FLOW_GAURD >= LINE_LENGTH)
	{
		fputs(data_set_line1, pFile);
		memset(data_set_line1, 0, LINE_LENGTH);
	}
	strcat(data_set_line1, ",");
	sprintf(data_set_line2, "%u", value);
	strcat(data_set_line1, data_set_line2);
}

static void print_container_from_tree_to_file(void* data, FILE *pFile, PDedup_data_set data_set)
{
	uint32 sn = *(uint32*)(data);
	Container cmpCont;
	cmpCont.sn = sn;
	PContainer conainter = avltree_find(&data_set->container_tree, &cmpCont);

	if (strlen(data_set_line1) + strlen(data_set_line2) + LINE_OVER_FLOW_GAURD >= LINE_LENGTH)
	{
		fputs(data_set_line1, pFile);
		memset(data_set_line1, 0, LINE_LENGTH);
	}
	strcat(data_set_line1, ",");
	sprintf(data_set_line2, "%u", sn);
	strcat(data_set_line1, data_set_line2);
	strcat(data_set_line1, ",");
	sprintf(data_set_line2, "%u", conainter->size);
	strcat(data_set_line1, data_set_line2);
}



int cmp_containers(const void* cnt1, const void* cnt2)
{
	PContainer pCnt1 = (PContainer)cnt1;
	PContainer pCnt2 = (PContainer)cnt2;

	return pCnt1->sn - pCnt2->sn;
}

Dedup_Error_Val dedup_data_set_init_args(PDedup_data_set data_set, char* file_name, uint32 containers_max_size, uint32 max_distance,
	uint32 max_pointers)
{
	char *file_name_no_type = malloc(MAX_FILE_NAME);
	char *num_string = malloc(MAX_FILE_NAME);

	memset(file_name_no_type, 0, MAX_FILE_NAME);
	memset(num_string, 0, MAX_FILE_NAME);
	memset(data_set, 0, sizeof(Dedup_data_set));

	data_set->max_distance_between_containers_for_file = max_distance;
	data_set->max_pointers_to_block = max_pointers;
	data_set->max_container_size = containers_max_size;
	data_set->num_of_active_systems = 0;
	data_set->num_of_systems = 0;

	/* Create the output file prefix and the error file name */
	strcpy(file_name_no_type, file_name);
	strtok(file_name_no_type, ".");
	strcpy(data_set->error_file_name, file_name_no_type);
	strcpy(data_set->output_file_name, file_name_no_type);
	memset(num_string, 0, MAX_FILE_NAME);
	sprintf(num_string, "_%u", containers_max_size);
	strcat(data_set->error_file_name, num_string);
	strcat(data_set->output_file_name, num_string);
	memset(num_string, 0, MAX_FILE_NAME);
	sprintf(num_string, "_D%u", max_distance);
	strcat(data_set->error_file_name, num_string);
	strcat(data_set->output_file_name, num_string);
	memset(num_string, 0, MAX_FILE_NAME);
	sprintf(num_string, "_P%u", max_pointers);
	strcat(data_set->error_file_name, num_string);
	strcat(data_set->output_file_name, num_string);
	strcat(data_set->error_file_name, ".err");

	free(file_name_no_type);
	free(num_string);

	return SUCCESS;
}

Dedup_Error_Val dedup_data_set_init_arrays(PDedup_data_set data_set, uint32 num_of_files, uint32 num_of_blocks, uint32 num_of_dirs)
{
	Dedup_Error_Val res = SUCCESS;

	data_set->num_of_dirs = num_of_dirs;
	// This is known in advance so we use malloc because this is only being done once.
	data_set->num_of_blocks = num_of_blocks;
	data_set->block_arr = calloc(num_of_blocks, sizeof(Block));

	if (data_set->block_arr == NULL)
	{
		return ALLOCATION_FAILURE;
	}


	data_set->num_of_files = num_of_files;
	data_set->file_arr = calloc(num_of_files + 1, sizeof(Dedup_File));
	if (data_set->file_arr == NULL)
	{
		free(data_set->block_arr);
		return ALLOCATION_FAILURE;
	}
	data_set->file_arr[num_of_files].sys_num = 0;

	avltree_init(cmp_containers, &(data_set->container_tree));

	return res;
}

Dedup_Error_Val dedup_data_set_destroy(PDedup_data_set data_set)
{
	assert(NULL != data_set);

	/*Destroy all blocks*/
	free(data_set->block_arr);

	/*Destroy all files*/
	free(data_set->file_arr);

	/*Destroy memory pool*/
	memory_pool_destroy(&data_set->mem_pool);

	/* del temp file */
	assert(remove(data_set->file_name_for_dir) == 0);

	return SUCCESS;
}


Dedup_Error_Val dedup_data_set_add_file(PDedup_data_set data_set, char* line, FILE* fptr)
{
	Dedup_Error_Val res = SUCCESS;

	/*
	lines in the input file may be extremly large, in need to check
	alot of end cases like lines ends with commas, line ends in the
	middle of a value etc.
	*/
	bool line_end_with_comma = false; //if the line ends with a comma it means we are in the middle of a line.
	char last_char = line[strlen(line) - 1];
	if (last_char == ',')
		line_end_with_comma = true;

	if (strcmp(strtok(line, ","), "F") != 0)//This is not a File line!
		return INVALID_ARGUMENT_FAILURE;

	uint32 sn = atoi(strtok(NULL, ","));

	char id[ID_LENGTH];
	strcpy(id, strtok(NULL, ","));

	char id_cpy[ID_LENGTH];
	strcpy(id_cpy, id);

	uint32 dir_sn = atoi(strtok(NULL, ","));

	uint32 block_amount = atoi(strtok(NULL, ","));

	res = dedup_file_init(&(data_set->file_arr[sn]), sn, 0, id, dir_sn, block_amount);
	assert(res == SUCCESS);

	/* The #blocks is known so we will not use dynamic array here */
	char* line_ptr;
	if (block_amount > 0) {

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
			assert(block_sn < data_set->num_of_blocks);
			block_size = atoi(line_ptr);
			data_set->block_arr[block_sn].size = block_size;

			PBlock_with_container block_w_container = NULL;
			res = memory_pool_alloc(&data_set->mem_pool, sizeof(Block_with_container), (uint32**)&block_w_container);
			assert(res == SUCCESS);
			block_w_container->block_sn = block_sn;
			avltree_add(&data_set->file_arr[sn].block_with_container_tree, block_w_container, &data_set->mem_pool);
		}
	}
	else
	{
		res = dedup_file_init(&(data_set->file_arr[sn]), REMOVED_SN, 0, id, dir_sn, 0);
		assert(res == SUCCESS);
	}
	if (block_amount > data_set->max_num_of_containers)
	{
		data_set->max_num_of_containers = block_amount;
	}

	uint32 sys_num = strtol(strtok(id_cpy, "_"),NULL,16);//atoi(strtok(id_cpy, "_"));
	data_set->file_arr[sn].sys_num = sys_num;

	return SUCCESS;
}

static void dedup_data_set_analyze_file_blocks_to_containers(void* data, PDedup_data_set data_set, uint32 curr_file_sn)
{
	Dedup_Error_Val ret_val = SUCCESS;
	PBlock curr_block;
	uint32 blockSize = 0, checkMaxSize = 0;
	uint32 *containers_filled = &(data_set->num_of_containers_filled);
	PBlock_with_container block_w_container = (PBlock_with_container)data;
	PContainer* curr_container = &data_set->curr_container;
	curr_block = &(data_set->block_arr[block_w_container->block_sn]);
	PDedup_File curr_file = &data_set->file_arr[curr_file_sn];

	bool not_in_container = curr_block->last_container_sn == BLOCK_NOT_IN_CONTAINER;
	bool max_distance_passed = (data_set->max_distance_between_containers_for_file != 0) && ((*containers_filled) - curr_block->last_container_sn > data_set->max_distance_between_containers_for_file);
	bool max_pointers_passed = (data_set->max_pointers_to_block != 0) && (curr_block->last_container_ref_count == data_set->max_pointers_to_block);
	/* For each block check if we need to insert it to the current container or not */

	blockSize = curr_block->size;
	if (not_in_container || max_distance_passed || max_pointers_passed)
	{
		/* We need to insert current block to the current container */
		checkMaxSize = (uint64)(*curr_container)->size + (uint64)curr_block->size;
		if (checkMaxSize > data_set->max_container_size || curr_block->last_container_sn == (*curr_container)->sn)
		{
			/* Current container cannot hold the block, lets open a new container
			* size is to big or to many pointers to block and current container already contains the block */
			if (data_set->max_container_size < curr_block->size)
			{
				FILE* error_file = NULL;
				if (!data_set->error_occured)
				{
					/*First error, lets create error file */
					error_file = fopen(data_set->error_file_name, "w");
					data_set->error_occured = true;
				}
				else
				{
					/*Not irst error, lets append to error file */
					error_file = fopen(data_set->error_file_name, "a");
				}
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

			create_container(curr_container, &(data_set->mem_pool), data_set->container_tree.count);
			avltree_add(&(data_set->container_tree), *curr_container, &(data_set->mem_pool));

			assert(ret_val == SUCCESS);
			(*containers_filled)++;

		}

		ret_val = container_add_file(*curr_container, &data_set->mem_pool, curr_file_sn);
		assert(ret_val == SUCCESS);

		ret_val = container_add_block(*curr_container, &data_set->mem_pool, block_w_container->block_sn, blockSize);
		assert(ret_val == SUCCESS);

		ret_val = block_add_container(curr_block, &data_set->mem_pool, *containers_filled);
		assert(ret_val == SUCCESS);

		/* we update the file that the current block is in the current container */
		block_w_container->container_sn = *containers_filled;
		uint32* container_sn_alloc = NULL;
		ret_val = memory_pool_alloc(&data_set->mem_pool, sizeof(uint32), (uint32**)&container_sn_alloc);
		assert(ret_val == SUCCESS);

		*container_sn_alloc = *containers_filled;
		avltree_add(&curr_file->container_tree, container_sn_alloc, &data_set->mem_pool);
	}
	else
	{
		/* Block is already in a container, lets update the container with the new file
		sn and the file with the container sn, also update continer ref count for block */
		PContainer temp;
		Container cmp_container = { 0 };
		cmp_container.sn = curr_block->last_container_sn;
		//ret_val = container_dynamic_array_get(container_arr, curr_block->last_container_sn, &temp);

		temp = avltree_find(&(data_set->container_tree), &cmp_container);
		assert(temp != NULL);

		ret_val = container_add_file(temp, &data_set->mem_pool, curr_file_sn);
		assert(ret_val == SUCCESS);
		ret_val = block_advance_last_container_ref_count(curr_block);
		assert(ret_val == SUCCESS);

		/* we update the file that the current block is in the current container */
		block_w_container->container_sn = curr_block->last_container_sn;
		uint32* container_sn_alloc = NULL;
		ret_val = memory_pool_alloc(&data_set->mem_pool, sizeof(uint32), (uint32**)&container_sn_alloc);
		assert(ret_val == SUCCESS);

		*container_sn_alloc = curr_block->last_container_sn;
		avltree_add(&curr_file->container_tree, container_sn_alloc, &data_set->mem_pool);
	}
}

Dedup_Error_Val dedup_data_set_analyze_to_containers(PDedup_data_set data_set)
{
	Dedup_Error_Val ret_val = SUCCESS;
	uint32 curr_file_sn;
	uint32 *containers_filled = &(data_set->num_of_containers_filled);
	uint64 checkMaxSize = 0;
	PDedup_File curr_file;
	uint64 container_index = 0;

	create_container(&data_set->curr_container, &(data_set->mem_pool), container_index);
	avltree_add(&(data_set->container_tree), data_set->curr_container, &(data_set->mem_pool));
	container_index++;

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
		avltree_for_each_block(&curr_file->block_with_container_tree, data_set, curr_file_sn, dedup_data_set_analyze_file_blocks_to_containers);
	}

	return SUCCESS;
}
void dedup_data_set_delete_files_blocks(void* data, PDedup_data_set data_set, uint32 curr_file_sn)
{
	Dedup_Error_Val ret = SUCCESS;
	PBlock curr_block;
	uint32 blockSize = 0, checkMaxSize = 0, curr_ref_count = 0;
	uint32 *containers_filled = &(data_set->num_of_containers_filled);
	PBlock_with_container block_w_container = (PBlock_with_container)data;
	PContainer curr_container = NULL;
	uint32 curr_block_sn = block_w_container->block_sn;
	uint32 curr_continer_sn = block_w_container->container_sn;
	Container cmpContainer;


	curr_block = &(data_set->block_arr[curr_block_sn]);

	cmpContainer.sn = curr_continer_sn;
	curr_container = avltree_find(&(data_set->container_tree), &cmpContainer);

	assert(curr_container != NULL);
	ret = container_del_file(curr_container, curr_file_sn);
	assert(ret == SUCCESS);
	ret = block_container_decrease_ref_count(curr_block, curr_continer_sn, &curr_ref_count);
	assert(ret == SUCCESS);
	if (curr_ref_count == INDEX_NOT_FOUND)
	{
		return;
	}

	if (curr_ref_count == 0)
	{
		if (curr_block->size > data_set->max_container_size)
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


	/*Loop over all the files*/
	while (curr_file_sn < data_set->num_of_files && curr_file->sys_num == system_sn)
	{
		avltree_for_each_block(&curr_file->block_with_container_tree, data_set, curr_file_sn, dedup_data_set_delete_files_blocks);

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

Dedup_Error_Val dedup_data_set_print_active_systems(PDedup_data_set data_set)
{

	Dedup_Error_Val res = SUCCESS;
	FILE *pFile;
	char file_name[MAX_FILE_NAME];
	memset(file_name, 0, MAX_FILE_NAME);
	strcpy(file_name, data_set->output_file_name);
	char deletion_order[MAX_FILE_NAME];
	memset(deletion_order, 0, MAX_FILE_NAME);

	uint32 *container_sns = calloc(data_set->max_num_of_containers, sizeof(uint32));
	assert(container_sns != NULL);
	if (data_set->num_of_systems - data_set->num_of_active_systems > 0)
	{
		strcat(file_name, "_R");
		for (uint32 i = 0; i < data_set->num_of_systems - data_set->num_of_active_systems; i++)
		{
			sprintf(deletion_order, "_%u", data_set->system_deletion_order[i]);
			strcat(file_name, deletion_order);
		}
	}
	strcat(file_name, ".csv");
	pFile = fopen(file_name, "w");
	assert(pFile != NULL);

	/*print header*/
	res = dedup_data_print_header(data_set, pFile);

	/*write files*/
	uint32 system_num;
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
	Container cmpContainer;

	for (container_sn = 0; container_sn <= data_set->num_of_containers_filled; container_sn++)
	{
		cmpContainer.sn = container_sn;
		pCurrentContainer = avltree_find(&(data_set->container_tree), &cmpContainer);

		assert(pCurrentContainer != NULL);
		res = dedup_data_print_container(data_set, pFile, pCurrentContainer);
		assert(res == SUCCESS);
	}

	/*write directories*/
	FILE *pTempFile = NULL;
	pTempFile = fopen(data_set->file_name_for_dir, "r");
	assert(pTempFile != NULL);
	char* prefix = NULL;
	uint32 system_sn = 0;
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
		system_sn = strtol(strtok(prefix, "_"), NULL, 16); //atoi(strtok(prefix, "_"));
		if (data_set->system_active[system_sn])
		{
			fputs(data_set_line1, pFile);
		}
		size_t curr_line_len = strlen(data_set_line1) - 1;
		while (data_set_line1[curr_line_len] != '\n')
		{
			line_ptr = fgets(data_set_line1, LINE_LENGTH, pTempFile);
			if (data_set->system_active[system_sn])
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
	sprintf(data_set_line2, "%u", data_set->num_of_containers_filled + 1);
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
	for (i = 0; i < data_set->num_of_systems - data_set->num_of_active_systems; i++)
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

	sprintf(data_set_line2, "%u", pDedup_file->container_tree.count);
	strcat(data_set_line1, data_set_line2);

	avltree_for_each_print_containers(&pDedup_file->container_tree, pFile, data_set, print_container_from_tree_to_file);

	strcat(data_set_line1, data_set_line3);

	strcat(data_set_line1, ",\n");
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

	avltree_for_each_print(&pContainer->file_array, pFile, print_sns_from_tree_to_file);

	strcat(data_set_line1, ",\n");
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

	avltree_for_each_print(&pContainer->block_array, pFile, print_sns_from_tree_to_file);

	strcat(data_set_line1, ",\n");
	fputs(data_set_line1, pFile);
	return SUCCESS;
}
