#include "vm/swap.h"
#include <bitmap.h>
#include <debug.h>
#include "devices/disk.h"
#include "threads/vaddr.h"
#include "filesys/file.h"

#define SWAP_CHANEL 1
#define SWAP_DEVICE 1

const int sectors_per_page = PGSIZE / DISK_SECTOR_SIZE;

struct bitmap *used_slots;
struct disk *swap_disk;
struct lock bitmap_lock;

void swap_init() {
	swap_disk = disk_get(SWAP_CHANEL, SWAP_DEVICE);
	used_slots = bitmap_create(disk_size(swap_disk) / sectors_per_page);
	lock_init(&bitmap_lock);
}

/* Copies page (which must be on the swap drive) into frame. */
void swap_in(struct page *page, struct frame *frame) {
	ASSERT(page->status == PAGE_SWAPPED);

	disk_sector_t start = page->swap_location;
	size_t swap_position = start / sectors_per_page;

	ASSERT(start % sectors_per_page == 0);
	ASSERT(bitmap_test(used_slots, swap_position));

	for(int i = 0; i < sectors_per_page; i++)
		disk_read(swap_disk, start + i, frame->addr + (i*DISK_SECTOR_SIZE));

	lock_acquire(&bitmap_lock);
	bitmap_flip(used_slots, swap_position);
	lock_release(&bitmap_lock);
	page->status = PAGE_OTHER;
}

/* Copies content of frame into a free swap slot */
void swap_out(struct frame *frame) {
	lock_acquire(&bitmap_lock);
	size_t swap_position = bitmap_scan_and_flip(used_slots, 0, 1, false);
	lock_release(&bitmap_lock);
	if(swap_position == BITMAP_ERROR)
		PANIC("All %d swap spots used.", bitmap_size(used_slots));
	disk_sector_t start = swap_position * sectors_per_page;

	for(int i = 0; i < sectors_per_page; i++)
		disk_write(swap_disk, start + i, frame->addr + (i*DISK_SECTOR_SIZE));

	frame->page->status = PAGE_SWAPPED;
	frame->page->swap_location = start;
}

void swap_discard(struct page *page) {
	ASSERT(page->status == PAGE_SWAPPED);
	disk_sector_t start = page->swap_location;
	size_t swap_position = start / sectors_per_page;

	ASSERT(start % sectors_per_page == 0);
	ASSERT(bitmap_test(used_slots, swap_position));

	bitmap_flip(used_slots, swap_position);
	page->status = PAGE_OTHER;
}
