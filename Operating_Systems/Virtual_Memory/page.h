#ifndef __VM_PAGE_H_
#define __VM_PAGE_H_

#include "threads/thread.h"
#include "filesys/file.h"
#include "devices/disk.h"

/* Supplemental page table */

/* Cap max stack size at 8MB. */
#define MAX_STACK_SIZE (8 * 1024 * 1024)
#define MIN_STACK_ADDR (PHYS_BASE - MAX_STACK_SIZE)

/* What type of a page is this */
enum page_type {
	PAGE_ONDISK,
	PAGE_SWAPPED,
	PAGE_MAPPED,
	PAGE_OTHER
};

/* Supplemental page table entry. */
struct page {
  void *vaddr; /* Virtual address of bottom of this page */
  enum page_type status; /* What type of page is this. */
  bool writeable; /* Can you write to the page? */
  struct lock moving; /* Is this page being moved in or out of mem. */
  bool deleting;

  /* Used by mmaped and ondisk files. */
  struct file *file; /* File page was loaded from. */
  off_t ofs; /* File offset of the bottom of the page. */
  size_t read_bytes; /* Number of bytes to read from the file. */
  size_t zero_bytes; /* Number of zero bytes in the file. */

  /* Used by swapped files. */
  disk_sector_t swap_location; /* Location of page on swap disk. */

  struct thread *thread; /* Thread that has this virtual address. */
  struct frame *current_frame; /* Frame that the page is loaded in (if any). */
  struct hash_elem elem; /* Used for hashing. */
};

void page_init(void);
struct page *page_alloc(void *, enum page_type);
void page_load(struct page *);
bool page_is_esp_valid(void *esp);
void page_destroy_all(void);
void page_destroy(struct page *page);
struct page *page_get_page(void *);

#endif /* __VM_PAGE_H_ */
