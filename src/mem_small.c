/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void *
emalloc_small(unsigned long size)
{
    if(arena.chunkpool == NULL){
        unsigned long count = mem_realloc_small() / CHUNKSIZE;
        void **ptr = (void **)arena.chunkpool;
        for(int i = 0; i < count - 1; i++){
            *ptr = (void *)((char *)ptr + CHUNKSIZE);
            ptr = *ptr;
        }
        *ptr = NULL;
        return emalloc_small(size);
    }
    void *ptr = arena.chunkpool;
    arena.chunkpool = *(void **) arena.chunkpool;
    return mark_memarea_and_get_user_ptr(ptr, CHUNKSIZE, SMALL_KIND);
}

void efree_small(Alloc a) {
    *(void **)a.ptr = arena.chunkpool;
    arena.chunkpool = a.ptr;
}
