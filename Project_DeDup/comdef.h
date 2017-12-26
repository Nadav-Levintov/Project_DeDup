#pragma once
#ifndef COMDEF_H
#define COMDEF_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ID_LENGTH 16
#define PROGRAM_ARG_SIZE 4 
#define MAX_SYSTEMS 128
#define DYNAMIC_ARRAY_SIZE 128
#define POOL_EXTEND_SIZE 128

typedef uint8_t uint8;
typedef uint32_t uint32;

typedef enum dedup_error_val_t
{
	SUCCESS,
	ALLOCATION_FAILURE,
	INVALID_ARGUMENT_FAILURE,
	DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR,
	UNKNOWN_FAILURE
} Dedup_Error_Val;

typedef struct folder_t
{
	uint32 sn;
	uint32 sys_num;
} Folder, *PFolder;

#endif // !COMDEF_H
