/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void * emalloc_small(unsigned long size) {
    // appelle la fonction de marquage en lui donnant l'adresse de l'element
    // sa taille CHUNCKSIZE et le type SMALL_KIND
        //= (char *)arena.chunkpool;
    if (arena.chunkpool  == NULL) {
        unsigned long block_size = mem_realloc_small();
        //char * addr = (char *) arena.chunkpool;
        //void **waddr = addr;
        void ** addr = arena.chunkpool;
        for(unsigned long i = 96; i < block_size; i+=96) {
            *addr = (char *)addr + 96;
            *addr += 96;
        }
        *addr = NULL;
    }
    void* valid_addr = arena.chunkpool;
    void** warena = arena.chunkpool;
    *warena = (unsigned long *) arena.chunkpool + 96/8;
    return mark_memarea_and_get_user_ptr(valid_addr, CHUNKSIZE, SMALL_KIND);
}

void efree_small(Alloc a) {
    *(void **) a.ptr = arena.chunkpool;
    arena.chunkpool = a.ptr;
}
