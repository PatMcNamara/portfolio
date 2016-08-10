#include "vm/page.h"
#include <hash.h>
#include <string.h>
#include <stdio.h>
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "vm/frame.h"
#include "vm/swap.h"

static bool install_page (void *upage, void *kpage, bool writable);

static hash_hash_func hash_member;
static hash_less_func cmp_member;
static hash_action_func destroy_hash_elem;
struct page *get_member(const struct hash_elem *);

/* Initializes the threads paging utilities.  Should be called before load. */
void page_init() {
	struct thread *t = thread_current();
	hash_init(&t->spt, hash_member, cmp_member, NULL);
	lock_init(&t->spt_lock);
}

/* Return's a new supplemental page that contains vaddr.  This page won't
   actually be put into a frame until a fault happens in the page. */
struct page *page_alloc(void *vaddr, enum page_type pg_t) {
	/* Allocate new page. */
	struct thread *t = thread_current();
	struct page *page = malloc(sizeof(struct page));

	/* Initialize page. */
	memset(page, 0, sizeof(struct page));
	page->vaddr = pg_round_down(vaddr);
	page->status = pg_t;
	page->zero_bytes = PGSIZE;
	page->thread = thread_current();
	lock_init(&page->moving);

	/* Insert page into page table. */
	lock_acquire(&t->spt_lock);
	ASSERT(hash_insert(&t->spt, &page->elem) == NULL);
	lock_release(&t->spt_lock);
	return page;
}

/* Loads given page into a frame. */
void page_load(struct page *page) {
	/* Make sure page isn't being evicted. */
	lock_acquire(&page->moving);

	ASSERT(pagedir_get_page(thread_current()->pagedir, page->vaddr) == NULL);
	ASSERT(page->current_frame == NULL);

	/* Get an empty frame. */
	struct frame *frame = frame_alloc();

	bool have_fslock = lock_held_by_current_thread(&fs_lock);
	frame->page = page;
  bool swapped = false;

  /* Load pages data into frame. */
  switch(page->status) {

  /* Load data from swap drive. */
  case PAGE_SWAPPED:
  	swap_in(page, frame);
  	swapped = true;
  	break;

  /* Load page from disk. */
  case PAGE_MAPPED:
  case PAGE_ONDISK:
  	/* Get file system lock. */
  	if(!have_fslock)
  		lock_acquire(&fs_lock);

  	/* Read data into frame. */
    if(file_read_at(page->file, frame->addr, page->read_bytes, page->ofs) !=
    	  (int) page->read_bytes)
    	PANIC("Failure reading from disk into frame, didn't read expcted %d bytes",
    			page->read_bytes);

    /* Release FS lock. */
    if(!have_fslock)
    	lock_release(&fs_lock);

    /* FALL THROUGH */
  /* Page is empty */
  default:
  	ASSERT(page->read_bytes + page->zero_bytes == PGSIZE);
  	memset(frame->addr + page->read_bytes, 0, page->zero_bytes);
  	break;
  }

  /* Add the page to the process's address space. */
  if (!install_page (page->vaddr, frame->addr, page->writeable))
  	PANIC("Failure adding mapping from uaddr %p and kaddr %p.", page->vaddr,
  			frame->addr);

	/* If no writes happen before frame is evicted, it must still be written
	   to swap. */
  if(swapped)
  	pagedir_set_dirty(thread_current()->pagedir, page->vaddr, true);

  page->current_frame = frame;
  lock_release(&page->moving);
  lock_release(&frame->evicting);
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}

/* Checks that stack is not greater then the max stack size (if it is, it
   will alloc pages such that any address above esp is valid). */
bool page_is_esp_valid(void *esp) {
	/* Is esp valid? */
	if(PHYS_BASE > esp && esp >= MIN_STACK_ADDR) {
		/* Create any needed pages. */
		for(; page_get_page(esp) == NULL; esp += PGSIZE) {
			ASSERT(esp < PHYS_BASE);
			page_alloc(esp, PAGE_OTHER)->writeable = true;
		}
		return true;
	}
	return false;
}


/* Destroy's all of this thread's pages and cleans up the thread's
   supplemental page table (should always be called before exiting a
   process). */
void page_destroy_all() {
	struct thread *t = thread_current();

	lock_acquire(&t->spt_lock);
	hash_destroy(&t->spt, destroy_hash_elem);
	lock_release(&t->spt_lock);
}

/* Destroys the given page.  Note that if this is called on a memory mapped
   page, all other pages in the same memory map will will be destroyed as
   well. Also note that only the first page of a memory mapping should be
   passed into this function. */
void page_destroy(struct page *page) {
	struct thread *t = thread_current();
	/* Memory mapped files are treated differently from all others. */
	if(page->status == PAGE_MAPPED) {
		struct page tmp;
		struct file *f = page->file;

		/* Start on last page of mmap. */
		lock_acquire(&fs_lock);
		tmp.vaddr = pg_round_down(page->vaddr + file_length(f));
		lock_release(&fs_lock);

		/* Iterate through pages, removing as you go. */
		lock_acquire(&t->spt_lock);
		for(; tmp.vaddr >= page->vaddr; tmp.vaddr -= PGSIZE) {
			destroy_hash_elem(hash_delete(&t->spt, &tmp.elem), NULL);
		}
		lock_release(&t->spt_lock);

		/* Close the file that the mmap was using. */
		lock_acquire(&fs_lock);
		file_close(f);
		lock_release(&fs_lock);
	} else {
		lock_acquire(&t->spt_lock);
		destroy_hash_elem(hash_delete(&t->spt, &page->elem), NULL);
		lock_release(&t->spt_lock);
	}
}

/* Hash action function that destroys a struct and free's it's struct. */
static void destroy_hash_elem(struct hash_elem *e, void *aux) {
	ASSERT(e != NULL);
	struct page *page = get_member(e);
	struct frame *f = page->current_frame;
	page->deleting = true;
	lock_acquire(&page->moving);

	/* Evict page if it's in a frame and descard the swap info if it's in swap. */
	if(f != NULL) {
	  if(lock_try_acquire(&f->evicting)) { /* Check if the frame is not being evicted. */
	    if(f->page == page) /* If you are still in the frame, free it. */
		    frame_free(f);
		  else /* If someone else is in the frame, do nothing. */
		    lock_release(&f->evicting);
		} else { /* Wait for eviction to finish. */
		  lock_acquire(&f->evicting);
		  lock_release(&f->evicting);
		}
  }
	if (page->status == PAGE_SWAPPED)
		swap_discard(page);

  ASSERT(list_empty(&page->moving.semaphore.waiters));

	free(page);
}

/* Get page struct that holds the address vaddr. */
struct page *page_get_page(void *vaddr) {//TODO start integrating this function all over, add in lock
  struct page page;
  page.vaddr = pg_round_down(vaddr);
  return hash_find(&thread_current()->spt, &page.elem) != NULL ? get_member(hash_find(&thread_current()->spt, &page.elem)) : NULL;
}
/* Page hash function (hashes on vaddr of page). */
unsigned hash_member(const struct hash_elem *e, void *aux) {
  struct page *member = get_member(e);
  return hash_int((int) pg_round_down(member->vaddr));
}
/* Hash comparison function for pages. */
bool cmp_member(const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED) {
  struct page *a = get_member(a_),
                    *b = get_member(b_);

  return a->vaddr < b->vaddr;
}
/* Gives the page struct that contains E. */
struct page *get_member(const struct hash_elem *e) {
  return hash_entry(e, struct page, elem);
}
