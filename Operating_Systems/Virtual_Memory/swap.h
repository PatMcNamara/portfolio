#ifndef __VM_SWAP_H_
#define __VM_SWAP_H_
#include "devices/disk.h"
#include "vm/frame.h"
#include "vm/page.h"

/* Swap table. */

void swap_init(void);
void swap_in(struct page *, struct frame *);
void swap_out(struct frame *);
void swap_discard(struct page *);

#endif /* __VM_SWAP_H_ */
