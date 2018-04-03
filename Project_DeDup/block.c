#include "block.h"

int cmp_container_with_ref(const void* cnt1, const void* cnt2)
{
	return ((PContainer_and_ref_count)cnt1)->container_sn - ((PContainer_and_ref_count)cnt2)->container_sn;
}

Dedup_Error_Val block_init(PBlock block, uint32 sn, char * id, uint32 shared_by_files)
{
	block->last_container_sn = BLOCK_NOT_IN_CONTAINER;
	block->last_container_ref_count = 0;
	block->sn = sn;
	strcpy(block->id, id);
	block->shared_by_files = shared_by_files;
	avltree_init(cmp_container_with_ref, &(block->container_with_ref_count_tree));
	return SUCCESS;
}

Dedup_Error_Val block_add_container(PBlock block, PMemory_pool pool, uint32 container_sn)
{
	Dedup_Error_Val ret_val = SUCCESS;
	PContainer_and_ref_count pValue = NULL;
	PContainer_and_ref_count pValue2 = NULL;

	ret_val = memory_pool_alloc(pool, sizeof(*pValue), (uint32**)&pValue);
	assert(ret_val == SUCCESS);


	pValue->container_sn = container_sn;
	pValue->ref_count = 1;

	/* Check if the array already holds container with the provided SN */
	pValue2 = avltree_add(&(block->container_with_ref_count_tree), pValue, pool);

	if (pValue2 == NULL)
	{
		/* New container, set last_st and ref count */
		block->last_container_sn = container_sn;
		block->last_container_ref_count = 1;
	}else
	{
		/* Old container, get the current refernce counter and increase it by 1 */
		pValue2->ref_count = pValue2->ref_count + 1;
		block->last_container_ref_count++;
	}
	return ret_val;
}

Dedup_Error_Val block_advance_last_container_ref_count(PBlock block)
{

	PContainer_and_ref_count pValue = NULL;
	Container_and_ref_count temp;
	temp.container_sn = block->last_container_sn;
	temp.ref_count = 1;

	/* Increase the ref count for the last continer to add the block */
	block->last_container_ref_count++;
	
	/* Update the continer with reference counter array accordingly */

	pValue = (PContainer_and_ref_count)avltree_find(&(block->container_with_ref_count_tree),(void*)&temp);
	assert(pValue != NULL);
	pValue->ref_count = pValue->ref_count + 1;
	
	
	return SUCCESS;
}

bool container_with_ref_array_dynamic_array_contains(avltree* tree, uint32 container_sn)
{
	PContainer_and_ref_count pValue = NULL;
	Container_and_ref_count value;
	value.container_sn = container_sn;
	value.ref_count = 1;
	pValue = (PContainer_and_ref_count)avltree_find(tree,(void*)&value);

	return pValue != NULL;
}

Dedup_Error_Val block_container_decrease_ref_count(PBlock block, uint32 container_sn, uint32* ref_count)
{
	Container_and_ref_count value;
	PContainer_and_ref_count pValue = NULL;
	avltree *tree = &(block->container_with_ref_count_tree);


	value.container_sn = container_sn;
	value.ref_count = 1;

	/* Find the continer*/
	pValue = (PContainer_and_ref_count)avltree_find(tree,(void*)&value);

	/* Find the continer in the array and update the ref count */
	if (pValue != NULL)
	{
		/* Get the ref count and decrease it */
		pValue->ref_count = pValue->ref_count - 1;
		*ref_count = pValue->ref_count ;
	}
	else
	{
		*ref_count = INDEX_NOT_FOUND;
	}

	return SUCCESS;
}
