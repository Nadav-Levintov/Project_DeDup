#include "dedup_file.h"

Dedup_Error_Val dedup_file_create(PDedup_File file, uint32 sn, uint8 sys_num, char * id, uint32 dir_sn, uint32 block_amount, PBlock_with_container block_with_container_array)
{
	file->sn = sn;
	file->sys_num;
	str_cpy(file->id,id);
	file->dir_sn=dir_sn;
	file->block_amount=block_amount;
	file->block_with_container_array = block_with_container_array;
	
	return SUCCESS;
}
