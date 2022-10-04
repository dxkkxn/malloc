/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include "mem.h"
#include "mem_internals.h"
#include <assert.h>
#include <stdint.h>

void *emalloc_small(unsigned long size) {
  // appelle la fonction de marquage en lui donnant l'adresse de l'element
  // sa taille CHUNCKSIZE et le type SMALL_KIND
  //= (char *)arena.chunkpool;
  if (arena.chunkpool == NULL) {
    unsigned long block_size = mem_realloc_small();
    void **addr = arena.chunkpool;
    assert(block_size % 96 == 0);
    for (unsigned long i = 0; i < (block_size / 96 - 1); i++) {
      *addr = (void *)((char *)addr + 96);
      addr = *addr;
    }
    *addr = NULL;
  }
  void *valid_addr = arena.chunkpool;
  arena.chunkpool = *(void **) arena.chunkpool;
  return mark_memarea_and_get_user_ptr(valid_addr, CHUNKSIZE, SMALL_KIND);
}

void efree_small(Alloc a) {
  *(void **)a.ptr = arena.chunkpool;
  arena.chunkpool = a.ptr;
}
