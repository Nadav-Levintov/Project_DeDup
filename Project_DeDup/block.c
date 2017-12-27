#include "block.h"

Dedup_Error_Val block_init(PBlock block, uint32 sn, char * id, uint32 shared_by_files)
{
	return SUCCESS;
}

Dedup_Error_Val block_add_container(PBlock block, PMemory_pool pool, uint32 container_sn)
{
	Dedup_Error_Val ret_val = SUCCESS;

	ret_val = dynamic_array_add(&(block->container_with_ref_count_array), pool, container_sn);
	assert(ret_val == SUCCESS);
	ret_val = dynamic_array_add(&(block->container_with_ref_count_array), pool, 1);
	assert(ret_val == SUCCESS);

	block->last_container_sn = container_sn;
	block->last_container_ref_count = 1;
	block->is_in_container = true;
	
	return ret_val;
}

Dedup_Error_Val block_advance_last_container_ref_count(PBlock block)
{
	
	Dedup_Error_Val ret_val = dynamic_array_update(&(block->container_with_ref_count_array), block->container_with_ref_count_array.length - 1, block->last_container_ref_count);
	assert(ret_val == SUCCESS);
	block->last_container_ref_count++;

	return ret_val;
}
