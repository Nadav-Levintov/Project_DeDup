#include "dedup_file.h"

static int cmp_block_with_container(const void* block1, const void* block2)
{
	return ((PBlock_with_container)block1)->block_sn - ((PBlock_with_container)block2)->block_sn;
}
static int cmp_containers_sns(const void* container1, const void* container2)
{
	return *(uint32*)container1 - *(uint32*)container2;
}

Dedup_Error_Val dedup_file_init(PDedup_File file, uint32 sn, uint8 sys_num, char * id, uint32 dir_sn, uint32 block_amount)
{
	file->sn = sn;
	file->sys_num = sys_num;
	strcpy(file->id,id);
	file->dir_sn=dir_sn;
	file->block_amount=block_amount;
	avltree_init(cmp_block_with_container, &(file->block_with_container_tree));
	avltree_init(cmp_containers_sns, &(file->container_tree));
	
	return SUCCESS;
}
