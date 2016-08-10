#ifndef __VM_FRAME_H_
#define __VM_FRAME_H_

#include <hash.h>
#include "threads/synch.h"

/* Frame table */
/* Globals used by clock algorithm (not yet implemented). */
struct list clocklist;
struct lock clocklock;
struct condition clockwaiting;
struct lock clockcondlock;
struct frame *clock_frame;
bool clockrecieved;
int clockwaiters;
bool clockmodified;

/* A frame entry. */
struct frame {
  void *addr; /* Pointer to the address of this frame (the allocated page). */
  struct page *page; /* Pointer to the page held in this frame. */
  struct lock evicting;
  struct hash_elem elem; /* Hash element. */
  struct list_elem clock_elem;
};

void frame_init(void);
struct frame *frame_alloc(void);
void frame_free(struct frame *);

#endif /* __VM_FRAME_H_ */
