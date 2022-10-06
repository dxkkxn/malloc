/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include "assert.h"
#include "mem.h"
#include "mem_internals.h"
#include <assert.h>
#include <stdint.h>

typedef enum { false, true } bool;

unsigned int puiss2(unsigned long size) {
  unsigned int p = 0;
  size = size - 1; // allocation start in 0
  while (size) {   // get the largest bit
    p++;
    size >>= 1;
  }
  if (size > (1 << p))
    p++;
  return p;
}

/*
 * Creates a new block of size 2**index and push it into the head
 * arena.TZL[index]
 */
void create_new_block(unsigned long index) {
  unsigned long max_i =
      FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
  if (index == max_i) {
    mem_realloc_medium();
    assert(arena.TZL[index] != NULL);
  } else {
    if (arena.TZL[index + 1] == NULL) {
      create_new_block(index + 1);
    }
    assert(arena.TZL[index + 1] != NULL);
    void *addr_to_divide = arena.TZL[index + 1];
    arena.TZL[index + 1] = *(void **)addr_to_divide;
    void **buddy = (void *)((uint64_t)addr_to_divide ^ (1 << index));
    *(void **)addr_to_divide = buddy;
    *buddy = arena.TZL[index];
    arena.TZL[index] = addr_to_divide;
  }
}

/*
 */
void *emalloc_medium(unsigned long size) {
  assert(size + 32 < LARGEALLOC);
  assert(size > SMALLALLOC);
  unsigned int index = puiss2(size + 32);
  if (arena.TZL[index] == NULL)
    create_new_block(index);
  assert(arena.TZL[index] != NULL);
  void *ptr = arena.TZL[index];
  arena.TZL[index] = *(void **)arena.TZL[index];
  return mark_memarea_and_get_user_ptr(ptr, size + 32, MEDIUM_KIND);
}

void *get_min_buddy(void *addr, unsigned long size) {
  void *min_addr;
  if ((uint64_t)addr < ((uint64_t)addr ^ size)) {
    min_addr = addr;
  } else {
    min_addr = (void *)((uint64_t)addr ^ size);
  }
  return min_addr;
}

void buddy_search(void *addr, unsigned long size) {
  // if buddy in tzl merge with the buddy recursively and return true
  // if not return false;
  // addr = address of the buddy
  void **curr = arena.TZL[size];
  void **prev = NULL;
  void *buddy_addr = (void *)((uint64_t)addr ^ (1 << size));
  while (curr != NULL) {
    if (curr == buddy_addr) {
      if (prev == NULL) {
        arena.TZL[size] = *curr;
      } else {
        *prev = *curr;
      }
      void *min_addr =
          (uint64_t)addr < (uint64_t)buddy_addr ? addr : buddy_addr;
      buddy_search(min_addr, size + 1);
      return;
    }
    prev = curr;
    curr = *curr;
  }
  // buddy not in TZL
  // insert addr in head
  *(void **)addr = arena.TZL[size];
  arena.TZL[size] = addr;
}
void efree_medium(Alloc a) {
  buddy_search(a.ptr, puiss2(a.size));
  return;
}
