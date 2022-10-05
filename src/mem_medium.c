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

void alloc_new_block(unsigned long size){
    unsigned long max = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    long diff = max - size;
    if(diff <= 0){       
        mem_realloc_medium();   
    }
    if(diff < 0){
        mem_realloc_medium(); 
        alloc_new_block(size + 1); 
    }
    if(arena.TZL[size] == NULL || diff != 0){
        alloc_new_block(size + 1);        
    }
    void *block_ptr = arena.TZL[size];
    
    //TZL[block_size] est remplacé par son compagnon s'il existe et est libéré sinon
    arena.TZL[size] = *(void **)arena.TZL[size];

    //construction du compagnon    
    void *buddy = (void *)((unsigned long)block_ptr ^ (1 << (size - 1)));
    *(void **)block_ptr = buddy;
    arena.TZL[size - 1] = block_ptr; 
}

void *
emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    unsigned long real_size = size + 32;
    unsigned int block_size = puiss2(real_size);
    if(arena.TZL[block_size] == NULL){
        alloc_new_block(block_size);
    }
    assert(arena.TZL[block_size] != NULL);
    void *ptr = arena.TZL[block_size];
    arena.TZL[block_size] = *(void **)arena.TZL[block_size];
    return mark_memarea_and_get_user_ptr(ptr, block_size, MEDIUM_KIND);

    return (void *) 0;
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
