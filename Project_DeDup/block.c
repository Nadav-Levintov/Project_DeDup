#include "block.h"

Dedup_Error_Val block_create(PBlock block, uint32 sn, char * id, uint32 shared_by_files)
{
	return SUCCESS;
}

Dedup_Error_Val block_set_size(PBlock block, uint32 block_size)
{
	block->size = block_size;
	return SUCCESS;
}
