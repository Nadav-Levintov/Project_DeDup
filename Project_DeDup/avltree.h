/*
 * avltree.h
 *
 *  Created on: 27 במרץ 2018
 *      Author: benny
 */

#ifndef PROJECT_DEDUP_AVLTREE_H_
#define PROJECT_DEDUP_AVLTREE_H_

#include <stdlib.h>
#include "comdef.h"
#include "memory_pool.h"

struct avltreenode
{
    struct avltreenode * left;
    struct avltreenode * right;
    struct avltreenode * parent;
    unsigned int leftheight;
    unsigned int rightheight;
    void * data;
};

typedef struct avltreenode avltreenode;

typedef int (*avltree_cmpfn)(const void*, const void*);
typedef void (*avltree_forfn)(void*);
typedef void(*avltree_for_print_func)(void*, FILE*);
typedef void(*avltree_for_print_conainers_func)(void*, FILE*, void*);
typedef void(*avltree_for_block)(void*, void*, uint32);

struct avltree {
    avltreenode * root;
    size_t count;
    avltree_cmpfn compare;
};

typedef struct avltree avltree;

avltree * avltree_create(avltree_cmpfn compare, PMemory_pool pool);
void avltree_delete(avltree * tree);
void avltree_for_each(const avltree * tree, avltree_forfn fun);
void avltree_for_each_block(const avltree * tree, void* data_set, uint32 curr_file_sn, avltree_for_block fun);
void avltree_for_each_print_containers(const avltree * tree, FILE *pFile, void* data_set, avltree_for_print_conainers_func fun);
void avltree_for_each_print(const avltree * tree, FILE *pFile, avltree_for_print_func fun);
void* avltree_add(avltree * tree, void * data, PMemory_pool pool);
void* avltree_find(const avltree * tree, const void* data);
void* avltree_remove(avltree * tree, const void* data);
void avltree_empty(avltree * tree);
size_t avltree_get_count(const avltree *tree);
void avltree_init(avltree_cmpfn compare, avltree *tree);

#endif /* PROJECT_DEDUP_AVLTREE_H_ */
