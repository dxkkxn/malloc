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

typedef enum{false, true} bool;

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
    if (size == max) {
        mem_realloc_medium();
        assert(arena.TZL[size] != NULL);
    }
    if (arena.TZL[size] == NULL) {
        create_new_block(size + 1);
    }
    void * addr_to_divide = arena.TZL[size];
    arena.TZL[size] = *(void **)arena.TZL[size];
    void ** buddy = (void *) ((uint64_t)addr_to_divide^(1<<(size - 1)));
    *(void **) addr_to_divide = buddy;
    *buddy = arena.TZL[size-1];
    arena.TZL[size-1] = addr_to_divide;
}


void * emalloc_medium(unsigned long size) {
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    unsigned int block_size = puiss2(size+32);
    if (arena.TZL[block_size] == NULL)
        create_new_block(block_size+1);
    assert(arena.TZL[block_size] != NULL);
    void * ptr = arena.TZL[block_size];
    arena.TZL[block_size] = *(void **) arena.TZL[block_size];
    return mark_memarea_and_get_user_ptr(ptr, size+32, MEDIUM_KIND);
}

void * get_min_buddy(void * addr, unsigned long size) {
    void * min_addr;
    if ((uint64_t) addr < ((uint64_t)addr ^ size)) {
        min_addr = addr;
    } else {
        min_addr =(void *)((uint64_t) addr ^ size);
    }
    return min_addr;
}

void buddy_search(void * addr, unsigned long size) {
    // if buddy in tzl merge with the buddy recursively and return true
    // if not return false;
    // addr = address of the buddy
    void ** curr = arena.TZL[size];
    void ** prev = NULL;
    void * buddy_addr = (void *)((uint64_t)addr ^ (1<<size));
    while (curr != NULL) {
        if (curr == buddy_addr) {
            if (prev == NULL) {
                arena.TZL[size] = *curr;
            } else {
                *prev = *curr;
            }
            void * min_addr = (uint64_t) addr < (uint64_t) buddy_addr ? addr : buddy_addr;
            buddy_search(min_addr, size+1);
            return;
        }
        prev = curr;
        curr = *curr;
    }
    // buddy not in TZL
    // insert addr in head
    *(void **) addr = arena.TZL[size];
    arena.TZL[size] = addr;
}
void efree_medium(Alloc a) {
    buddy_search(a.ptr, puiss2(a.size));
    return;
}
