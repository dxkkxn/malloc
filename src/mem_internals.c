/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include "mem_internals.h"
#include "mem.h"
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>

unsigned long knuth_mmix_one_round(unsigned long in) {
  return in * 6364136223846793005UL % 1442695040888963407UL;
}

void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k) {
  unsigned long *long_ptr = (unsigned long *)ptr;
  *long_ptr = (unsigned long)size;
  unsigned long magic = knuth_mmix_one_round((unsigned long)long_ptr);
  long_ptr++;
  magic &= ~(0b11UL);
  magic |= k;
  *long_ptr = magic;
  long_ptr++;
  unsigned long *usable_memory = long_ptr;

  char * char_ptr = (char *)ptr;
  char_ptr += size - 16;
  long_ptr = (unsigned long *)char_ptr;
  *long_ptr = magic;
  long_ptr++;
  *long_ptr = size;
  return (void *)usable_memory;
}

Alloc mark_check_and_get_alloc(void *ptr) {
  unsigned long *long_ptr = (unsigned long *)ptr;
  long_ptr -= 2;
  void *start_ptr = (void *)(long_ptr);
  unsigned long size = *long_ptr;
  unsigned long magic = *(long_ptr + 1);
  MemKind kind = magic & 0b11UL;
  magic &= ~(0b11UL); // pop 2 last bits
  unsigned long magic_verif = knuth_mmix_one_round((unsigned long)long_ptr);
  magic_verif &= ~(0b11UL); // pop 2 last bits
  assert(magic == magic_verif);
  // verif of last block
  char * char_ptr = (char *)start_ptr;
  char_ptr += size - 16;
  long_ptr = (unsigned long *)char_ptr;
  unsigned long magic_last = *long_ptr;
  unsigned long size_last = *(long_ptr + 1);
  assert(size == size_last);
  size_last;
  MemKind kind_last = magic_last & 0b11UL;
  magic_last &= ~(0b11UL); // pop 2 last bits
  assert(magic_last == magic);
  assert(kind == kind_last);

  Alloc a = {start_ptr, kind, size};
  return a;
}

unsigned long mem_realloc_small() {
  assert(arena.chunkpool == 0);
  unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
  arena.chunkpool = mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (arena.chunkpool == MAP_FAILED)
    handle_fatalError("small realloc");
  arena.small_next_exponant++;
  return size;
}

unsigned long mem_realloc_medium() {
  uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
  assert(arena.TZL[indice] == 0);
  unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
  assert(size == (1 << indice));
  arena.TZL[indice] = mmap(0,
                           size * 2, // twice the size to allign
                           PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (arena.TZL[indice] == MAP_FAILED)
    handle_fatalError("medium realloc");
  // align allocation to a multiple of the size
  // for buddy algo
  arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
  arena.medium_next_exponant++;
  return size; // lie on allocation size, but never free
}

// used for test in buddy algo
unsigned int nb_TZL_entries() {
  int nb = 0;

  for (int i = 0; i < TZL_SIZE; i++)
    if (arena.TZL[i])
      nb++;

  return nb;
}
