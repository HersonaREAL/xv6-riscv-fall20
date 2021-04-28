// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define hashsz 13

extern uint ticks;

struct {
  struct spinlock lock;
  struct buf buf[NBUF];


  //bucket
  struct bucket {
    struct buf head;
  } bk[hashsz];

  //lock for each bucket
  struct spinlock bucketlock[hashsz];
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  //struct buf head;
} bcache;



static void hash_init() {
  //init bucket
  for (int i = 0; i < hashsz;++i) {
    bcache.bk[i].head.prev = &bcache.bk[i].head;
    bcache.bk[i].head.next = &bcache.bk[i].head;
  }
}

static void hash_put(uint ori_blockno,uint blockno,struct buf* bufptr) {
  uint pos = blockno % hashsz;
  uint pos1 = ori_blockno % hashsz;

  //aviod dead lock
  if(pos1!=pos)
    acquire(&bcache.bucketlock[pos1]);
  
  //unlink from ori bucket
  bufptr->next->prev = bufptr->prev;
  bufptr->prev->next = bufptr->next;

  if(pos1!=pos)
    release(&bcache.bucketlock[pos1]);

  //link to new bucket
  bufptr->next = bcache.bk[pos].head.next;
  bufptr->prev = &bcache.bk[pos].head;
  bcache.bk[pos].head.next->prev = bufptr;
  bcache.bk[pos].head.next = bufptr;
}

static struct buf* hash_get(uint blockno,uint dev) {
  uint pos = blockno % hashsz;
  struct buf *b;
  for(b = bcache.bk[pos].head.next; b != &bcache.bk[pos].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      return b;
    }
  }
  return 0;
}





void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");
  char *lockname = "bcache.bk";
  for (int i = 0; i < hashsz;++i) {
    initlock(&bcache.bucketlock[i], lockname);
  }

  hash_init();
  // Create linked list of buffers
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    //add all buf to first bucket
    b->next = bcache.bk[0].head.next;
    b->prev = &bcache.bk[0].head;
    initsleeplock(&b->lock, "buffer");
    b->ticks = ticks;
    bcache.bk[0].head.next->prev = b;
    bcache.bk[0].head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  acquire(&bcache.bucketlock[blockno % hashsz]);
  b = hash_get(blockno, dev);
  if(b) {
    b->refcnt++;
    //release(&bcache.lock);
    release(&bcache.bucketlock[blockno % hashsz]);
    acquiresleep(&b->lock);
    return b;
  }
 
  //find least recently used unused buffer
  acquire(&bcache.lock);
  uint t = ticks;
  for(struct buf *tmp = bcache.buf; tmp < bcache.buf+NBUF; tmp++) {
    if(tmp->refcnt==0&&tmp->ticks<=t) {
      b = tmp;
      t = tmp->ticks;
    }
  }
  if(b) {
    uint ori_blockno = b->blockno;

    b->dev = dev;
    b->blockno = blockno;
    b->valid = 0;
    b->refcnt = 1;
    release(&bcache.lock);


    //put the buf to the appropriate bucket
    hash_put(ori_blockno,blockno, b);

    release(&bcache.bucketlock[blockno % hashsz]);

    acquiresleep(&b->lock);
    return b;
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  //acquire(&bcache.lock);
  int idx = b->blockno % hashsz;
  acquire(&bcache.bucketlock[idx]);
  b->refcnt--;
  if (b->refcnt == 0) {
    //just change ticks
    b->ticks = ticks;
  }
  release(&bcache.bucketlock[idx]);
  //release(&bcache.lock);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


