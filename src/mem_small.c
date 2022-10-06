/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include "mem.h"
#include "mem_internals.h"
#include <assert.h>
#include <stdint.h>

//get the head of the list then return its mark result
void *emalloc_small(unsigned long size) {
  if (arena.chunkpool == NULL) { //build the list if it's empty
    unsigned long block_size = mem_realloc_small();
    void **addr = arena.chunkpool;
    assert(block_size % CHUNKSIZE == 0);
    for (unsigned long i = 0; i < (block_size / CHUNKSIZE - 1); i++) {
      *addr = (void *)((char *)addr + CHUNKSIZE); //each element points to the next one
      addr = *addr;
    }
    *addr = NULL;
  }
  void *valid_addr = arena.chunkpool; //store the head in valid_addr
  arena.chunkpool = *(void **) arena.chunkpool; //the following element becomes the head
  return mark_memarea_and_get_user_ptr(valid_addr, CHUNKSIZE, SMALL_KIND);
}

//change the head of the list
void efree_small(Alloc a) {
  *(void **)a.ptr = arena.chunkpool; //a.ptr points to the (previous) head
  arena.chunkpool = a.ptr; //a.ptr is now the new head
} 
