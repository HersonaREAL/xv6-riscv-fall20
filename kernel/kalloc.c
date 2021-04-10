// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

// struct {
//   struct spinlock lock;
//   struct run *freelist;
// } kmem;

struct {
  struct spinlock lock;
  struct run *freelist;
} ckmem[NCPU];

int getcpuid() {
  push_off();
  int res = cpuid();
  pop_off();
  return res;
}

void
kinit()
{
  //for lock of each cpu list
  for (int i = 0; i < NCPU; ++i) {
    initlock(&ckmem[i].lock, "kmem");
  }
  
  //initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int cpuid;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  //get cpu id
  cpuid = getcpuid();


  //operate cpu free list
  acquire(&ckmem[cpuid].lock);

  r->next = ckmem[cpuid].freelist;
  ckmem[cpuid].freelist = r;

  release(&ckmem[cpuid].lock);

}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int cpuid;
  //get cpu id
  cpuid = getcpuid();



  acquire(&ckmem[cpuid].lock);
  r = ckmem[cpuid].freelist;
  //has page?
  if(r)
    ckmem[cpuid].freelist = r->next;
  else {

    //steal mem from other cpus
    for (int i = 0; i < NCPU;++i) {

      //check left page
      if(ckmem[i].freelist) {
        acquire(&ckmem[i].lock);

        //check again
        if(ckmem[i].freelist) {
          
          //just steal one page
          r = ckmem[i].freelist;
          ckmem[i].freelist = r->next;

          release(&ckmem[i].lock);
          break;
        }
        else
          release(&ckmem[i].lock);
      }
    }
  }
  release(&ckmem[cpuid].lock);


  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
