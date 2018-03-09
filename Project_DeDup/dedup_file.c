#include "dedup_file.h"

Dedup_Error_Val dedup_file_create(PDedup_File file, uint32 sn, uint8 sys_num, char * id, uint32 dir_sn, uint32 block_amount, PBlock_with_container block_with_container_array)
{
	file->sn = sn;
	file->sys_num = sys_num;
	strcpy(file->id,id);
	file->dir_sn=dir_sn;
	file->block_amount=block_amount;
	file->block_with_container_array = block_with_container_array;
	
	return SUCCESS;
}

bool dedup_file_ContainsCurrentBlock(PDedup_File curr_file, uint32 block_sn, uint32 max_index)
{
	uint32 index = 0;

	assert(curr_file != NULL);

	for(index = 0; index < max_index; index++)
	{
		if(curr_file->block_with_container_array[index].block_sn == block_sn)
		{
			return true;
		}
	}
	return false;
}
