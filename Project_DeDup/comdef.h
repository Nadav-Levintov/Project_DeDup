#pragma once
#ifndef COMDEF_H
#define COMDEF_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define ID_LENGTH (32)
#define LINE_LENGTH (1024*32)
#define PROGRAM_ARG_SIZE (5)
#define MAX_SYSTEMS (1024)
#define MAX_FILE_NAME (256)
#define DYNAMIC_ARRAY_SIZE (128)
#define POOL_INITIAL_SIZE (128 * 1024 * 1024)
#define POOL_EXTEND_SIZE (32*1024)
#define REMOVED_SN (0xFFFFFFFFU)
#define INDEX_NOT_FOUND (0xFFFFFFFFU)
#define BLOCK_CONTAINER_POOL_INITIAL_SZIZE (1024*1024*128)
#define LINE_OVER_FLOW_GAURD 5

typedef uint8_t uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;

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

/* Calculates x^y */
static inline uint32 pow_aux(uint32 x, uint32 y)
{
	if (y == 0)
	{
		return 1;
	}
	uint32 res = x;
	uint32 i;
	for (i = 1; i < y; i++)
	{
		res *= x;
	}

	return res;
}

#endif // !COMDEF_H
