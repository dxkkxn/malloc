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
    *buddy = arena.TZL[index]; // buddy points to the first element in index
    arena.TZL[index] = addr_to_divide; // addr_to_divide becomes the new head
  }
}

void *emalloc_medium(unsigned long size) {
  assert(size + 32 < LARGEALLOC); // The limit
  assert(size > SMALLALLOC);
  unsigned int index = puiss2(size + 32);
  if (arena.TZL[index] == NULL)
    create_new_block(index);
  assert(arena.TZL[index] != NULL);
  void *ptr = arena.TZL[index];
  arena.TZL[index] = *(void **)arena.TZL[index];
  return mark_memarea_and_get_user_ptr(ptr, size + 32, MEDIUM_KIND);
}

/*
 * Search the buddy in arena.TZL[index] if it founds it search the buddy of the
 * new block recursively
 */
void buddy_search(void *addr, unsigned long index) {
  void **curr = arena.TZL[index];
  void **prev = NULL;
  void *buddy_addr = (void *)((uint64_t)addr ^ (1 << index));
  while (curr != NULL) {
    if (curr == buddy_addr) {
      if (prev == NULL) {
        // buddy in the head of linked list
        arena.TZL[index] = *curr;
      } else {
        // gets out the block found
        *prev = *curr;
      }
      void *min_addr =
          (uint64_t)addr < (uint64_t)buddy_addr ? addr : buddy_addr;
      // get the smaller adresss and search for the buddy in larger block
      buddy_search(min_addr, index + 1);
      return;
    }
    prev = curr;
    curr = *curr;
  }
  // buddy not in TZL
  // insert addr in head at arena.TZL[index]
  *(void **)addr = arena.TZL[index];
  arena.TZL[index] = addr;
}
void efree_medium(Alloc a) {
  buddy_search(a.ptr, puiss2(a.size));
  return;
}
