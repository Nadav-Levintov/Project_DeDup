#pragma once
#ifndef DEDUP_CONTAINERS_H
#define DEDUP_CONTAINERS_H

#include <stdio.h>
#include <stdlib.h>
#include "comdef.h"
#include "dedup_data_set.h"

/*
	@Function:	parse_file
	
	@Params:	Name of the file to parse
				Pointer to the data set that will hold the parsed data
	
	@Desc:		Parse the .csv file in to deduplicatin containers
*/
Dedup_Error_Val parse_file(char* file_name, PDedup_data_set data_set);

/*
	@Function:	parse_header
	
	@Params:	Pointer to the file descriptor to parse
				Pointer to the data set that will hold the parsed data
				Pointer to the buffer to be used for reading from the file.

	
	@Desc:		Parse the header of the .csv file
*/
Dedup_Error_Val parse_header(FILE *fd, PDedup_data_set data_set, char * line);

/*
	@Function:	print_data_set
	
	@Params:	Pointer to the data set that will hold the parsed data
		
	@Desc:		Print the data set to a .csv file
*/
Dedup_Error_Val print_data_set(PDedup_data_set data_set);

/*
@Function:	user_interaction

@Params:	Pointer to the data set that will hold the parsed data

@Desc:		Recive instructions from the user
*/
Dedup_Error_Val user_interaction(PDedup_data_set data_set);

#endif // !DEDUP_CONTAINERS_H
