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