#pragma once
#ifndef DEDUP_DATA_SET_H
#define DEDUP_DATA_SET_H

#include <string.h>
#include "comdef.h"
#include "dedup_file.h"
#include "block.h"
#include "container.h"
#include "memory_pool.h"
#include "avltree.h"

typedef struct dedup_data_set_t
{
	uint32 num_of_containers_filled;
	uint32 num_of_files;
	uint32 num_of_blocks;
	uint32 num_of_dirs;
	uint32 max_distance_between_containers_for_file;
	uint32 max_pointers_to_block;
	uint32 max_container_size; // in bytes
	uint32 system_file_index[MAX_SYSTEMS];
	uint32 system_deletion_order[MAX_SYSTEMS];
	uint32 num_of_systems;
	uint32 num_of_active_systems;
	uint32 max_num_of_containers;
	uint32 max_pointers_per_container;
	uint32 avg_pointers_per_container;
	PDedup_File file_arr;
	PBlock block_arr;
	PContainer curr_container;
	//Container_dynamic_array container_arr;
	avltree container_tree;
	Memory_pool mem_pool;
	char file_name_for_dir[MAX_FILE_NAME];
	char error_file_name[MAX_FILE_NAME];
	char output_file_name[MAX_FILE_NAME];
	bool system_active[MAX_SYSTEMS];
	bool error_occured;
	bool is_block_file; //Blocks = TRUE, Physical = FALSE

} Dedup_data_set, *PDedup_data_set;

/*
	@Function:	dedup_data_set_init_args
	
	@Params:	data_set -	Pointer to the data set.
				file_name -	The input file name.
				max_pointers - The maximum pointers allowed to point to a block before a new copy is created.
				containers_max_size -	maximum size of for each container in bytes.
				max_distance -	Maximum distance (in containers) allowed between last time we've put the block
					in a container and the current container.
	
	@Desc:	Initialize the data set with the provided arguemnts.
*/
Dedup_Error_Val dedup_data_set_init_args(PDedup_data_set data_set, char* file_name, uint32 max_pointers, uint32 containers_max_size,
	uint32 max_distance);

/*
	@Function:	dedup_data_set_init_arrays
	
	@Params:	data_set -	Pointer to the data set.
				num_of_files -	Number of files in the input file (according to the file header).
				num_of_blocks - Number of blocks in the input file (according to the file header).
				num_of_dirs -	Number of directories in the input file (according to the file header).
	
	@Desc:	Initialize the data set arrays with the information extracted from the input file header.
*/
Dedup_Error_Val dedup_data_set_init_arrays(PDedup_data_set data_set, uint32 num_of_files, uint32 num_of_blocks, uint32 num_of_dirs);

/*
	@Function:	dedup_data_set_add_file
	
	@Params:	data_set -	Pointer to the data set.
				line -	current buffer line.
				fptr - Pointer to the input file descriptor.
	
	@Desc:	Add a file from fptr opend file according to the line in the buffer, may read more lines if required.
*/
Dedup_Error_Val dedup_data_set_add_file(PDedup_data_set data_set, char* line, FILE* fptr);

/*
	@Function:	dedup_data_set_add_block
	
	@Params:	data_set -	Pointer to the data set.
				line -	current buffer line.
				fptr - Pointer to the input file descriptor.
	
	@Desc:	Add a block from fptr opend file according to the line in the buffer, may read more lines if required.
*/
Dedup_Error_Val dedup_data_set_add_block(PDedup_data_set data_set, char* line, FILE* fptr);

/*
	@Function:	dedup_data_set_analyze_to_containers
	
	@Params:	data_set -	Pointer to the data set.
	
	@Desc:	Analyze the data set and create containers.
*/
Dedup_Error_Val dedup_data_set_analyze_to_containers(PDedup_data_set data_set);

/*
	@Function:	dedup_data_set_delete_system
	
	@Params:	data_set -	Pointer to the data set.
				system_sn -	Serial number of the system to be deleted.

	@Desc:	Remove all files of the specified system from the data set.
*/
Dedup_Error_Val dedup_data_set_delete_system(PDedup_data_set data_set, uint32 system_sn);

/*
	@Function:	dedup_data_set_print_active_systems
	
	@Params:	data_set -	Pointer to the data set.
	
	@Desc:	Print all active systems in the data set to a file.
*/
Dedup_Error_Val dedup_data_set_print_active_systems(PDedup_data_set data_set);

/*
	@Function:	dedup_data_set_destroy
	
	@Params:	data_set -	Pointer to the data set.
	
	@Desc:	Destroy and free memory used by the data set.
*/
Dedup_Error_Val dedup_data_set_destroy(PDedup_data_set data_set);

/*
	@Function:	dedup_data_print_dfile
	
	@Params:	data_set -	Pointer to the data set.
				pFile -		Pointer to the file descriptor of the output file.
				pDedup_file -	Pointer to the file (from input file) to be printed to the output file.
				container_sns - Array of the serial numbers of the containers that has blocks of the file
	
	@Desc:	Print file line in the output file.
*/
Dedup_Error_Val dedup_data_print_dfile(PDedup_data_set data_set, FILE *pFile, PDedup_File pDedup_file, uint32 * container_sns);

/*
	@Function:	dedup_data_print_container
	
	@Params:	data_set -	Pointer to the data set.
				pFile -		Pointer to the file descriptor of the output file.
				pContainer -	Pointer to the container to be printed to the output file.
				container_sns - Array of the serial numbers of the containers that has blocks of the file
	
	@Desc:	Print conatiner lines in the output file.
*/
Dedup_Error_Val dedup_data_print_container(PDedup_data_set data_set, FILE *pFile, const PContainer pContainer);

/*
	@Function:	dedup_data_print_container
	
	@Params:	data_set -	Pointer to the data set.
				pFile -		Pointer to the file descriptor of the output file.
	
	@Desc:	Print the header of the output file.
*/
Dedup_Error_Val dedup_data_print_header(PDedup_data_set data_set, FILE *pFile);

#endif // !DEDUP_DATA_SET_H
