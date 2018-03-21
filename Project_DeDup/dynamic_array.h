#pragma once
#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "comdef.h"
#include "memory_pool.h"

typedef struct dynamic_array_t
{
	uint32 length;
	uint32 arr[DYNAMIC_ARRAY_SIZE];
	uint32 max_val, min_val;
	struct dynamic_array_t* next_arr;
}Dynamic_array, *PDynamic_array;

/*
	@Function:	dynamic_array_get
	
	@Params:	head -	Pointer to the head of the dynamic array.
				index -	The index in the array to retrive.
				res -	Pointer that will hold the returned value.
	
	@Desc:		res will hold the value from the requested index from the array that starts with head.
				If the array is smaller than index an DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR error value will be returned.
*/
Dedup_Error_Val dynamic_array_get(PDynamic_array head, uint32 index, uint32* res);

/*
	@Function:	dynamic_array_update
	
	@Params:	head -	Pointer to the head of the dynamic array.
				index -	The index in the array to update.
				val -	The new value to place in cell in place index.
	
	@Desc:		The array[index] will be updated with the new value.
				If the array is smaller than index an DYNAMIC_ARRAY_OUT_OF_BOUNDS_ERROR error value will be returned.
*/
Dedup_Error_Val dynamic_array_update(PDynamic_array head, uint32 index, uint32 val, bool update_min_max);

/*
	@Function:	dynamic_array_add
	
	@Params:	head -	Pointer to the head of the dynamic array.
				pool -	Pointer to the memory pool from which to allocate more memory if required.
				val -	The new value to add to the array.
	
	@Desc:		Value val will be added to the end of the array, if required the array will be increased by
				DYNAMIC_ARRAY_SIZE cells from the pool.
*/
Dedup_Error_Val dynamic_array_add(PDynamic_array head, PMemory_pool pool,  uint32 val, bool update_min_max);

/*
	@Function:	dynamic_array_contains
	
	@Params:	head -	Pointer to the head of the dynamic array.
				val -	The new value to add to the array.
				index -	Pointer to the variable that will hold the index of the value in the array.
	
	@Desc:		Check if dynamic array that starts with head contains the value val, if it is the index will be
				returned in the index pointer.

	@Return:	true -	if array contains val.
				false -	if the array does not contain val.
*/
bool dynamic_array_contains(PDynamic_array head,  uint32 val, uint32* index);


#endif // !DYNAMIC_ARRAY_H
