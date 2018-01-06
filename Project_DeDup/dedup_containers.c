#include "dedup_containers.h"

int main(int argc, char *argv[]) {
	if (argc != PROGRAM_ARG_SIZE)
	{
		printf("Wrong input.\n");
		printf("Input foramt: ./dedup_containers <input file path> <Max distance between containers per file(0 for no limit)> <Max pointers per block (0 for no limit)> \n");
		exit(1);
	}
	Dedup_data_set data_set;

	dedup_data_set_init_args(&data_set, atoi(argv[2]), atoi(argv[3]));

	return 0;
}

Dedup_Error_Val parse_file(char * file_name, PDedup_data_set data_set)
{
	Dedup_Error_Val res = SUCCESS;
	FILE *fptr, *dir_temp_file ;
	strcpy(data_set->file_name_for_dir, strtok(file_name, "."));
	strcat(data_set->file_name_for_dir, "_temp_file" );

	fptr = fopen(file_name, "r");
	assert(fptr != NULL);

	dir_temp_file = fopen(data_set->file_name_for_dir, "a");
	assert(fptr != NULL);



	/*Read header of file*/
	res = parse_header(fptr, data_set);

	char line[LENGTH_LENGHT];

	/* loop over file and read first letter and activate the relevant function */

	while (fgets(line, sizeof(line), fptr))
	{
		char* prefix = strtok(line, ",");
		if (strcmp(prefix, "F") == 0)
		{
			dedup_data_set_add_file(data_set, line);
		}
		else if (strcmp(prefix, "P") == 0 || strcmp(prefix, "B") == 0)
		{
			dedup_data_set_add_block(data_set, line);
		}
		else if (strcmp(prefix, "D") == 0)
		{
			fprintf(dir_temp_file, "%s", line);
		}
	}

	
	fclose(fptr);
	fclose(dir_temp_file);

	return res;
}

Dedup_Error_Val parse_header(FILE * fd, PDedup_data_set data_set)
{
	char line[LENGTH_LENGHT];

	uint32 line_index = 0;
	while (fgets(line, sizeof(line), fd) && line[0] == '#')
	{
		char* prefix = strtok(line, ":");
		if (strcmp(prefix, "#Num files") == 0)
			data_set->num_of_files = atoi(strtok(line, ":"));
		if (strcmp(prefix, "#Num directories") == 0)
			data_set->num_of_dirs = atoi(strtok(line, ":"));
		if (strcmp(prefix, "#Num blocks") == 0 || strcmp(prefix, "#Num physical files") == 0)
			data_set->num_of_blocks = atoi(strtok(line, ":"));
		line_index++;
	}
	return SUCCESS;
}

Dedup_Error_Val print_data_set(PDedup_data_set data_set, char *fileName)
{
	FILE *fptr;

	fptr = fopen(fileName, "r");
	assert(fptr != NULL);

	return dedup_data_set_print_active_systems(data_set, fptr);

	fclose(fptr);
}

Dedup_Error_Val delete_system(PDedup_data_set data_set, uint32 system_num)
{
	//TODO
	return SUCCESS;
}

Dedup_Error_Val read_user_input(PDedup_data_set data_set)
{
	//TODO
	return SUCCESS;
}
