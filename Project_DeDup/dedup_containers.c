#include "dedup_containers.h"
char containers_line1[LINE_LENGTH];
char containers_line2[LINE_LENGTH];
Dedup_data_set data_set;

int main(int argc, char *argv[]) {
	Dedup_Error_Val res;

	if (argc != PROGRAM_ARG_SIZE)
	{
		printf("Wrong input.\n");
		printf("Input foramt: ./dedup_containers <input file path> <Max container size> <Max distance between containers per file(0 for no limit)> <Max pointers per block (0 for no limit)> \n");
		exit(1);
	}

	// init the data structure
	dedup_data_set_init_args(&data_set, argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

	//Read file
	res = parse_file(argv[1], &data_set);
	assert(res == SUCCESS);

	//Insert data into containers
	res = dedup_data_set_analyze_to_containers(&data_set);
	assert(res == SUCCESS);

	//Wait for user input
	user_interaction(&data_set);
	assert(res == SUCCESS);

	return 0;
}

Dedup_Error_Val parse_file(char* file_name, PDedup_data_set data_set)
{
	Dedup_Error_Val res = SUCCESS;
	FILE *fptr, *dir_temp_file;
	char temp_file_name[MAX_FILE_NAME];
	strcpy(temp_file_name, file_name);
	strcpy(data_set->file_name_for_dir, strtok(temp_file_name, "."));
	strcat(data_set->file_name_for_dir, "_temp_file");

	fptr = fopen(file_name, "r");
	assert(fptr != NULL);

	dir_temp_file = fopen(data_set->file_name_for_dir, "w");
	assert(fptr != NULL);

	/*Read header of file*/
	memset(containers_line1, 0, LINE_LENGTH);
	memset(containers_line2, 0, LINE_LENGTH);
	res = parse_header(fptr, data_set, containers_line1);

	assert(res == SUCCESS);
	char* line_ptr = containers_line1;

	/* loop over file and read first letter and activate the relevant function */
	while (line_ptr)
	{
		strcpy(containers_line2, containers_line1);
		char* prefix = strtok(containers_line2, ",");
		if (strcmp(prefix, "F") == 0)
		{
			res = dedup_data_set_add_file(data_set, containers_line1, fptr);
			assert(res == SUCCESS);
		}
		else if (strcmp(prefix, "P") == 0 || strcmp(prefix, "B") == 0)
		{
			res = dedup_data_set_add_block(data_set, containers_line1, fptr);
			assert(res == SUCCESS);
		}
		else if ((strcmp(prefix, "D") == 0) || (strcmp(prefix, "R") == 0))
		{
			strcpy(containers_line2, containers_line1);
			fputs(containers_line2, dir_temp_file);
			size_t curr_line_len = strlen(containers_line1) - 1;
			while (containers_line1[curr_line_len] != '\n')
			{
				line_ptr = fgets(containers_line1, LINE_LENGTH, fptr);
				strcpy(containers_line2, containers_line1);
				fputs(containers_line2, dir_temp_file);
				curr_line_len = strlen(containers_line1) - 1;
			}
		}
		line_ptr = fgets(containers_line1, LINE_LENGTH, fptr);
	}


	fclose(fptr);
	fclose(dir_temp_file);

	return res;
}

Dedup_Error_Val parse_header(FILE * fd, PDedup_data_set data_set, char * header_line)
{

	uint32 line_index = 0;
	uint32 num_of_files = 0, num_of_dirs = 0, num_of_blocks = 0;
	while (fgets(header_line, LINE_LENGTH, fd) && header_line[0] == '#')
	{
		strcpy(containers_line2, header_line);

		char* prefix = strtok(containers_line2, ":");
		char* val = strtok(NULL, "\n");
		if (strcmp(prefix, "# Output type") == 0)
			if (strcmp(val, " block-level") == 0)
				data_set->is_block_file = true;
		if (strcmp(prefix, "# Num files") == 0)
			num_of_files = atoi(val);
		if (strcmp(prefix, "# Num directories") == 0)
			num_of_dirs = atoi(val);
		if (strcmp(prefix, "# Num blocks") == 0 || strcmp(prefix, "# Num physical files") == 0)
			num_of_blocks = atoi(val);
		line_index++;
	}
	assert(num_of_files != 0);
	assert(num_of_dirs != 0);
	assert(num_of_blocks != 0);

	return dedup_data_set_init_arrays(data_set, num_of_files, num_of_blocks, num_of_dirs);
}

Dedup_Error_Val print_data_set(PDedup_data_set data_set)
{
	return dedup_data_set_print_active_systems(data_set);
}


Dedup_Error_Val user_interaction(PDedup_data_set data_set)
{
	Dedup_Error_Val res = SUCCESS;
	char command_buffer[1024];
	char* cmd;

	while (true)
	{
		printf("Please write what you wish to do: \n");
		printf("delete <system numbers seperated by spaces>\n");
		printf("print \n");
		printf("exit \n");
		fflush(stdout);
		scanf(" %[^\n]s", (char*)&command_buffer);
		cmd = strtok(command_buffer, " ");
		/*option 3*/
		if (strcmp(cmd, "exit") == 0)
		{
			res = dedup_data_set_destroy(data_set);
			printf("Good bye\n");
			return res;
		}

		/*option 1*/
		if (strcmp(cmd, "delete") == 0)
		{
			cmd = strtok(NULL, " ");
			if (cmd == NULL)
			{
				printf("No systems were entered for deletion!\n");
			}
			while (cmd != NULL)
			{
				res = dedup_data_set_delete_system(data_set, atoi(cmd));
				assert(res == SUCCESS);
				printf("System %u was deleted\n", atoi(cmd));
				cmd = strtok(NULL, " ");
			}
		}
		else if (strcmp(cmd, "print") == 0)
		{
			/*option 2*/
			res = print_data_set(data_set);
			assert(res == SUCCESS);
			printf("Data was printed to file\n");
		}
		else
		{
			printf("Illegal command\n");
		}

	}

	return res;
}
