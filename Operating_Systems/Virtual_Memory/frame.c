#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "vm/swap.h"

#include <random.h>
#include <stdio.h>
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "devices/timer.h"

struct hash frame_table;
struct lock ft_lock;

struct frame *get_frame_for_eviction(void);
void evict(struct frame *);

struct frame *get_frame(const struct hash_elem *);
hash_hash_func hash_frame;
hash_less_func cmp_frame;

/* Used by clock algorithm. */
//#define CLOCK
static thread_func clock_daemon;
inline static struct frame *get_clock_frame(struct list_elem *);
static struct frame *get_next_clock_frame(struct frame *);

/* Initializes the frame table. */
void frame_init() {
  hash_init(&frame_table, hash_frame, cmp_frame, NULL);
  random_init(0);
  lock_init(&ft_lock);

  /* Used by clock. */
  lock_init(&clocklock);
  list_init(&clocklist);
  cond_init(&clockwaiting);
  lock_init(&clockcondlock);
  cond_init(&clockwaiting);
  clockrecieved = true;
  clockwaiters = 0;
  clockmodified = true;
#ifdef CLOCK
  thread_create("clock.d", PRI_DEFAULT, clock_daemon, NULL);
#endif
}

/* Allocates an unused user frame.  If there  */
struct frame *frame_alloc() {
  struct frame *f = NULL;
  
  lock_acquire(&ft_lock);
  void *pg = palloc_get_page(PAL_USER);

  if(pg == NULL) {
  	/* If there are no frames left, evict a page. */
    f = get_frame_for_eviction();
    lock_release(&ft_lock);
    lock_acquire(&f->page->moving);//TODO this should never block. need to use a monitor instead to synch between the evictor and call to page_free()
    evict(f);
  } else {
  	/* Initialize new frame. */
    f = malloc(sizeof(struct frame));
    f->addr = pg;
    lock_init(&f->evicting);
    lock_acquire(&f->evicting);

    /* Add frame to frame table. */
    hash_insert(&frame_table, &f->elem);
    lock_release(&ft_lock);
  }
  return f;
}

#ifdef CLOCK
struct frame *get_frame_for_eviction() {//FIXME Implement clock
	struct frame *f;
//	printf("getting lock\n");
	lock_acquire(&clockcondlock);
	clockwaiters++;
//	printf("waiting\n");
	cond_wait(&clockwaiting, &clockcondlock);
	f = clock_frame;
	clockrecieved = true;
	clockwaiters--;
	lock_release(&clockcondlock);
	return f;
}
#else
/* Randomly sellects a page to evict. */
struct frame *get_frame_for_eviction() {
	struct hash_iterator hi;
	struct hash_elem e;

  ASSERT(lock_held_by_current_thread(&ft_lock));

//	lock_acquire(&ft_lock);
	do {
		long index = random_ulong() % hash_size(&frame_table);
		hash_first(&hi, &frame_table);
		for(int i = 0; i < index; i++)
			hash_next(&hi);
	} while(!lock_try_acquire(&get_frame(hash_cur(&hi))->evicting));
	struct frame *f = get_frame(hash_cur(&hi));
	ASSERT(!f->page->deleting);
//	lock_release(&ft_lock);
	
  return f;
}
#endif

/* CLOCK ALGORITHM NOT IMPLEMENTED */
#define CLOCK_DIFF_COUNT 100
#define CLOCK_WAIT_MS 100

static void clock_daemon(void *aux) {
//	printf("starting clock\n");
	struct frame *head, *tail;
	bool got_frame = false;
/*	while(true)
		thread_yield();*/
//	sema_down(&clockwaiting);
//	int i;
	while(1) {
//		printf("locking\n");
		lock_acquire(&clocklock);
//		printf("Checking list is empty\n");
		if(list_empty(&clocklist)) {
//			printf("Releasing lock\n");
			lock_release(&clocklock);
//			printf("Sleeping\n");
			timer_msleep(CLOCK_WAIT_MS);
//			thread_yield();
//			printf("Woke up\n");
			continue;
		}
//		printf("List is not empty\n");
		if(clockmodified) {
//			printf("clockmodified");
			head = tail = get_clock_frame(list_begin(&clocklist));
//			i = 0;
		}
//		printf("clock got lock\n");
		for(int i = 0; i < CLOCK_DIFF_COUNT; i++) {
//			printf("in for loop");
//			if(!clockmodified)
//				i = CLOCK_WAIT_COUNT;
//			printf("clock on frame %p: checking semaphore at %p\n", current, &current->evicting);
			/* Clear head's accessed bit. */
			if(lock_try_acquire(&head->evicting)) {
//				printf("\t\tgot head sema\n");
				pagedir_set_accessed(head->page->thread->pagedir, head->page->vaddr, false);//TODO check if it is being moved
				lock_release(&head->evicting);
			}
			/* If clock hands have reached proper angle, use tail hand. */
			if(!clockmodified/*i >= CLOCK_WAIT_COUNT*/) {
				/* Check if frame has been changed accessed since head passed. */
				if(lock_try_acquire(&tail->evicting)) {
					if(!pagedir_is_writeable/*accessed*/(tail->page->thread->pagedir, tail->page->vaddr)) {
						/* Wait for clock frame to be cleared then set it to tail. */
//						if(sema_try_down(&clockwaiting))
						lock_acquire(&clockcondlock);
						if(clockwaiters > 0 && clockrecieved) {
							printf("Found non accessed dir\n");
							clock_frame = tail;
							clockrecieved = false;
							got_frame = true;
							cond_signal(&clockwaiting, &clockcondlock);
						}
						lock_release(&clockcondlock);
//							sema_up(&clockready);
//						break;
						//						}
					}
					if(!got_frame) {
						lock_release(&tail->evicting);
					} else {
						got_frame = true;
					}
				}
				tail = get_next_clock_frame(tail);
			}
//			printf("getting next frame\n");
			head = get_next_clock_frame(head);
//			printf("sleeping\n");
		}
		clockmodified = false;
		lock_release(&clocklock);
		timer_msleep(CLOCK_WAIT_MS);
//		thread_yield();
	}
	PANIC("Frame eviction daemon quit!\n");
}

inline static struct frame *get_clock_frame(struct list_elem *e) {
	return list_entry(e, struct frame, clock_elem);
}
static struct frame *get_next_clock_frame(struct frame *f) {
	struct list_elem *e = list_next(&f->clock_elem);
	return get_clock_frame((e == list_end(&clocklist) ?
													  e : list_begin(&clocklist)));
}
/* \CLOCK\ */

/* Evicts the page held in F. */
void evict(struct frame *f) {
	uint32_t *pd = thread_current()->pagedir;
    
  ASSERT(lock_held_by_current_thread(&f->evicting));
//  lock_acquire(&f->page->moving); /* You should never block here. */
  ASSERT(f->page != NULL);
  ASSERT(lock_held_by_current_thread(&f->page->moving));

	/* Clear page to prevent further writes. */
	pagedir_clear_page(f->page->thread->pagedir, f->page->vaddr);

	/* If the page had been written to, the changes must be saved. */
	if(pagedir_is_dirty(f->page->thread->pagedir, f->page->vaddr)) {
		if(f->page->status == PAGE_MAPPED) {
			/* If the page is mapped, write changes back to file. */
			bool have_lock = lock_held_by_current_thread(&fs_lock);

			if(!have_lock)
				lock_acquire(&fs_lock);
			if(file_write_at(f->page->file, f->addr, f->page->read_bytes, f->page->ofs) != f->page->read_bytes)
				PANIC("Didn't write expected %d bytes.", f->page->read_bytes);
			if(!have_lock)
				lock_release(&fs_lock);

		} else {
			/* Write page to swap. */
			ASSERT(pagedir_is_writeable(f->page->thread->pagedir, f->page->vaddr));
			ASSERT(f->page->writeable);
			swap_out(f);
		}
	}

	/* Unset link the page and frame. */
	f->page->current_frame = NULL;
	lock_release(&f->page->moving);
	f->page = NULL;
}

/* Free's all data associated with the frame with kernel virtual address PG. */
void frame_free(struct frame *f) {
  struct hash_elem *e;
  ASSERT(lock_held_by_current_thread(&f->evicting));

  /* Delete frame from frame table. */
  lock_acquire(&ft_lock);
  ASSERT(hash_delete(&frame_table, &f->elem) != NULL);

	/* Evict page from frame. */
 	if(f->page != NULL)
	  evict(f);
  		
  ASSERT(list_empty(&f->evicting.semaphore.waiters));

	/* Free resources */
	palloc_free_page(f->addr);
  free(f);
  lock_release(&ft_lock);
}

/* Frame hash function (hashes on frame kernel addr). */
unsigned hash_frame(const struct hash_elem *h, void *aux) {
	return hash_int((int) get_frame(h)->addr);
}
/* Frame hash comparison function. */
bool cmp_frame(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
	return get_frame(a)->addr < get_frame(b)->addr;
}
/* Return's frame that holds the hash elem H. */
struct frame *get_frame(const struct hash_elem *h) {
  return hash_entry(h, struct frame, elem);
}
