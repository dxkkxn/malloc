/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"
#include "assert.h"

unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}

void create_new_block(unsigned long size) {
    unsigned long max = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    void* divided;
    if (size == max) {
        mem_realloc_medium();
        assert(arena.TZL[size] != NULL);
        //void ** addr = arena.TZL[size];
        divided = arena.TZL[size];
        arena.TZL[size] = (void*)((unsigned long) arena.TZL[size] ^ size);
        arena.TZL[size-1] = divided;
    }

    if (arena.TZL[size] == NULL) {
        create_new_block(size + 1);
        assert(arena.TZL[size] != NULL);
        //void ** addr = arena.TZL[size];
        divided = arena.TZL[size];
        arena.TZL[size] = (void*)((unsigned long) arena.TZL[size] ^ size);
        arena.TZL[size-1] = divided;
        return;
    }
    return;
}


void * emalloc_medium(unsigned long size) {
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    unsigned int block_size = puiss2(size+32);
    if (arena.TZL[block_size] == NULL)
        create_new_block(block_size);
    assert(arena.TZL[block_size] != NULL);
    void * ptr = arena.TZL[block_size];
    //arena.TZL[block_size] = *ptr;
    return mark_memarea_and_get_user_ptr(ptr, size+32, MEDIUM_KIND);
}


void efree_medium(Alloc a) {

    return;
}
