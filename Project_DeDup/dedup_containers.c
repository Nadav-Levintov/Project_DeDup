#include "dedup_containers.h"

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

	dedup_data_set_init_args(&data_set, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

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
	char line[LINE_LENGTH];
	res = parse_header(fptr, data_set, line);

	assert(res == SUCCESS);

	char curr_line[LINE_LENGTH];
	char* line_ptr = line;

	/* loop over file and read first letter and activate the relevant function */
	while (line_ptr)
	{
		strcpy(curr_line, line);
		char* prefix = strtok(curr_line, ",");
		if (strcmp(prefix, "F") == 0)
		{
			res = dedup_data_set_add_file(data_set, line, fptr);
			assert(res == SUCCESS);
		}
		else if (strcmp(prefix, "P") == 0 || strcmp(prefix, "B") == 0)
		{
			res = dedup_data_set_add_block(data_set, line, fptr);
			assert(res == SUCCESS);
		}
		else if (strcmp(prefix, "D") == 0)
		{
			strcpy(curr_line, line);
			fputs(curr_line, dir_temp_file);
			int curr_line_len = strlen(line) - 1;
			while (line[curr_line_len] != '\n')
			{
				line_ptr = fgets(line, LINE_LENGTH, fptr);
				strcpy(curr_line, line);
				fputs(curr_line, dir_temp_file);
				curr_line_len = strlen(line) - 1;
			}
		}
		line_ptr = fgets(line, LINE_LENGTH, fptr);
	}


	fclose(fptr);
	fclose(dir_temp_file);

	return res;
}

Dedup_Error_Val parse_header(FILE * fd, PDedup_data_set data_set, char * line)
{

	uint32 line_index = 0;
	uint32 num_of_files = 0, num_of_dirs = 0, num_of_blocks = 0;
	while (fgets(line, LINE_LENGTH, fd) && line[0] == '#')
	{
		char curr_line[LINE_LENGTH];
		strcpy(curr_line, line);

		char* prefix = strtok(curr_line, ":");
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

Dedup_Error_Val print_data_set(PDedup_data_set data_set, char *fileName)
{
	return dedup_data_set_print_active_systems(data_set, fileName);
}


Dedup_Error_Val user_interaction(PDedup_data_set data_set)
{
	Dedup_Error_Val res = SUCCESS;
	char command_buffer[1024];

	while (true)
	{
		printf("Please write what you wish to do: \n");
		printf("option 1: delete_system <system number>\n");
		printf("option 2: print_all <file name> \n");
		printf("option 3: destroy \n");
		scanf(" %[^\n]s", &command_buffer);

		/*option 3*/
		if (strcmp(command_buffer, "destroy") == 0)
		{
			res = dedup_data_set_destroy(data_set);
			printf("Good bye\n");
			return res;
		}

		/*option 1*/
		if (strcmp(command_buffer, "delete_system") == 0)
		{
			scanf(" %[^\n]s", &command_buffer);
			res = dedup_data_set_delete_system(data_set, atoi(command_buffer));

			assert(res == SUCCESS);
			printf("System %u was deleted\n", atoi(command_buffer));
		}
		else if (strcmp(command_buffer, "print_all") == 0)
		{
			/*option 2*/
			scanf(" %[^\n]s", &command_buffer);
			res = print_data_set(data_set, command_buffer);
			assert(res == SUCCESS);
			printf("Data was printed to %s\n", command_buffer);
		}
		else
		{
			printf("Illegal command\n");
		}

	}

	return res;
}
