#include "dedup_containers.h"

int main(int argc, char *argv[]) {


	uint32 max_distance_between_containers_for_file;
	uint32 max_pointers_to_block;
	uint32 max_container_size;
	Dedup_Error_Val res;
	Dedup_data_set data_set;

	if (argc != PROGRAM_ARG_SIZE)
	{
		printf("Wrong input.\n");
		printf("Input foramt: ./dedup_containers <input file path> <Max container size> <Max distance between containers per file(0 for no limit)> <Max pointers per block (0 for no limit)> \n");
		exit(1);
	}

	// init the data structure
	dedup_data_set_init_args(&data_set, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

	//Read file
	res =  parse_file(&argv[1], &data_set, max_container_size, max_pointers_to_block, max_distance_between_containers_for_file);
	assert(res == SUCCESS);

	//Insert data into containers
	res = dedup_data_set_analyze_to_containers(data_set);
	assert(res == SUCCESS);

	//Wait for user input
	user_interaction(&data_set);
	assert(res == SUCCESS);

	return 0;
}

Dedup_Error_Val parse_file(char* file_name, PDedup_data_set data_set)
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

	assert(res == SUCCESS);

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
	uint32 num_of_files = 0, num_of_dirs = 0, num_of_blocks = 0;
	while (fgets(line, sizeof(line), fd) && line[0] == '#')
	{
		char* prefix = strtok(line, ":");
		if (strcmp(prefix, "#Num files") == 0)
			num_of_files = atoi(strtok(line, ":"));
		if (strcmp(prefix, "#Num directories") == 0)
			num_of_dirs = atoi(strtok(line, ":"));
		if (strcmp(prefix, "#Num blocks") == 0 || strcmp(prefix, "#Num physical files") == 0)
			num_of_blocks = atoi(strtok(line, ":"));
		line_index++;
	}
	assert(num_of_files != 0);
	assert(num_of_dirs != 0);
	assert(num_of_blocks != 0);

	return dedup_data_set_init_arrays(data_set, num_of_files, num_of_blocks, num_of_dirs);
}

Dedup_Error_Val print_data_set(PDedup_data_set data_set, char *fileName)
{
	FILE *fptr;

	fptr = fopen(fileName, "r");
	assert(fptr != NULL);

	return dedup_data_set_print_active_systems(data_set, fptr);

	fclose(fptr);
}


Dedup_Error_Val user_interaction(PDedup_data_set data_set)
{
	Dedup_Error_Val res = SUCCESS;
	char command_buffer[1024];

	while(true)
	{
	    printf("Please write what you wish to do: /n");
	    printf("option 1: delete_system <system number>/n");
	    printf("option 2: print_all <file name> /n");
	    printf("option 3: destroy /n");
	    scanf("%[^\n]s", &command_buffer);

	    /*option 3*/
	    if(strcmp(command_buffer,"destroy")==0)
	    {
	    	res = dedup_data_set_destroy(data_set);
	    	printf("Good buy\n");
	    	return res;
	    }

	    /*option 1*/
	    if(strcmp(command_buffer,"delete_system") == 0)
	    {
	    	scanf("%[^\n]s", &command_buffer);
	    	res = dedup_data_set_delete_system(data_set, atoi(command_buffer));

	    	assert(res == SUCCESS);
	    	printf("System %d was deleted\n",  atoi(command_buffer));
	    }else if(strcmp(command_buffer,"print_all") == 0)
	    {
	    	/*option 2*/
	    	scanf("%[^\n]s", &command_buffer);
	    	res = dedup_data_set_print_active_systems(data_set, command_buffer);
	    	assert(res == SUCCESS);
	    	printf("Data was printed to %s\n", command_buffer);
	    }else
	    {
	    	printf("Illegal command/n");
	    }

	}

	return res;
}
