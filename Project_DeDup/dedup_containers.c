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

	fptr = fopen(file_name, "r");
	assert(fptr != NULL);
	fptr = fopen(strcat(strtok(file_name, "."), "temp_dir" ), "a");
	assert(fptr != NULL);
	// open file

	/*Read header of file*/
	Dedup_Error_Val parse_header(fptr, data_set);

	char line[LENGTH_LENGHT];

	// loop over file and read first letter and activate the relevant function

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
	while (fgets(line, sizeof(line), fd) && line_index < 5)
	{
		char* prefix = strtok(line, ":");
		if (strcmp(prefix, "#Num files") == 0)
			data_set->num_of_files = atoi(strtok(line, ":"));
		if (strcmp(prefix, "#Num directories") == 0)
			data_set->num_of_dirs = atoi(strtok(line, ":"));
		if (strcmp(prefix, "#Num blocks") == 0 || strcmp(prefix, "#Num physical files") == 0)
			data_set->num_of_blocks = atoi(strtok(line, ":"));
	}
	return SUCCESS;
}
