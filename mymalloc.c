#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>

typedef struct _mblock_t {
  struct _mblock_t * prev;
  struct _mblock_t * next;
  size_t size;
  int status;
  void * payload;
} mblock_t;

#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload)

typedef struct _mlist_t {
    mblock_t * head;
} mlist_t;

mlist_t mlist = {NULL};

void * mymalloc(size_t size);
void myfree(void * ptr);
mblock_t * findLastMemlistBlock();
mblock_t * findFreeBlockOfSize(size_t size);
void splitBlockAtSize(mblock_t * block, size_t newSize);
void coallesceBlockPrev(mblock_t * freedBlock);
void coallesceBlockNext(mblock_t * freedBlock);
mblock_t * growHeapBySize(size_t size);
void printMemList(const mblock_t* headptr);


int main(int argc, char* argv[]) {
  void * p1 = mymalloc(10);
  void * p2 = mymalloc(100);
  void * p3 = mymalloc(200);
  void * p4 = mymalloc(500);

  myfree(p3); p3 = NULL;
  myfree(p2); p2 = NULL;

  void * p5 = mymalloc(150);
  void * p6 = mymalloc(500);

  myfree(p4); p4 = NULL;
  myfree(p5); p5 = NULL;
  myfree(p6); p6 = NULL;
  myfree(p1); p1 = NULL;
}

void * mymalloc(size_t size) {

}

void myfree(void * ptr) {

}

// return the last block in memlist, used when we extend the heap with sbrk
mblock_t * findLastMemlistBlock() {
  if (mlist.head == NULL) {
    return NULL;
  }

  mblock_t * curr = mlist.head;
  while (curr->next != NULL) {
    curr = curr->next;
  }

  return curr;
}

// find the first free block that can hold the desired size
mblock_t * findFreeBlockOfSize(size_t size) {
  mblock_t * curr = mlist.head;  
  
  while (curr != NULL) {
    if (curr->status == 0 && curr->size >= size) { // if memory block is both free and large enough
      return curr;
    }
    else {
      curr = curr->next;
    }
  }

  return NULL; // if none found
}

// when an mblock is not fully used, create a new block and place it after the given one
void splitBlockAtSize(mblock_t * block, size_t newSize) {
    size_t totalSize = block->size;
    size_t neededSize = newSize + MBLOCK_HEADER_SZ;

    // if the mblock is not large enought to split into two, just allocate the whole mblock
    if (totalSize < needed + MBLOCK_HEADER_SZ + 1) {
      block->status = 1;
      return;
    }

    // otherwise, initialize a new mblock after the given mblock
    char * newBlockAddr = (char *)block + MBLOCK_HEADER_SZ + newSize;
    mblock_t * newBlock = (mblock_t *) newBlockAddr;
    newBlock->prev = block;
    newBlock->next = block->next;
    newBlock->status = 0; // set to free
    newBlock->size = totalSize - newSize - MBLOCK_HEADER_SZ;
    newBlock->payload = (void *)(newBlockAddr + MBLOCK_HEADER_SZ);

    // update the next block
    if (block->next != NULL) {
      block->next->prev = newBlock;
    }
    
    // update the given block
    block->next = newBlock
    block->size = newSize;
    block->status = 1; // set to allocated
}

// merge given mblock with the previous mblock
void coallesceBlockPrev(mblock_t * freedBlock) {
  // if previous block exists and is free, merge
  if (freedBlock->prev != NULL && freedBlock->prev->status == 0) {
    // expand prevBlock
    mblock_t * prevBlock = freedBlock->prev;
    prevBlock->size = prevBlock->size + freedBlock->size + MBLOCK_HEADER_SZ;
    
    // remove freedBlock from the chain
    prevBlock->next = freedBlock->next;
    if (freedBlock->next != NULL) {
      freedBlock->next->prev = prevBlock;
    }
  }
}

// merge given mblock with the previous mblock
void coallesceBlockNext(mblock_t * freedBlock) {
  // if next block exists and is free, merge
  if (freedBlock->next != NULL && freedBlock->next->status == 0) {
    // expand freedBlock
    mblock_t * nextBlock = freedBlock->next;
    freedBlock->size = freedBlock->size + nextBlock->size + MBLOCK_HEADER_SZ;
    
    // remove nextBlock from the chain
    freedBlock->next = nextBlock->next;
    if (nextBlock->next != NULL) {
      nextBlock->next->prev = freedBlock;
    }
  }
}

// expand the head to fit a new allocated block
mblock_t * growHeapBySize(size_t size) {
  size_t minSize = 1024; // 1Kb minimum
  size_t totalSize = size + MBLOCK_HEADER_SZ;

  if (totalSize < minSize) {
    totalSize = minSize;
  }

  // request memory from the OS
  void * oldBrk = sbrk(totalSize);
  if (oldBrk == (void *) -1) {
    return NULL; // sbrk failed
  }

  // convert the returned address to mblock
  mblock_t * newBlock = (mblock_t *) oldBrk;
  newBlock->prev = NULL;
  newBlock->next = NULL;
  newBlock->size = totalSize - MBLOCK_HEADER_SZ;
  newBlock->status = 0; // set to free
  newBlock->payload = (void *)((char *)newBlock + MBLOCK_HEADER_SZ);

  // attach mblock to mlist
  if (mlist.head == NULL) {
    mlist.head = newBlock;
  }
  else {
    mblock_t * last = findLastMemlistBlock();
    last->next = newBlock;
    newBlock->prev = last;
  }

  return newBlock;
}

// for testing purposes, copied from hw instructions
void printMemList(const mblock_t* head) {
  const mblock_t* p = head;
  
  size_t i = 0;
  while(p != NULL) {
    printf("[%ld] p: %p\n", i, (void*)p);
    printf("[%ld] p->size: %ld\n", i, p->size);
    printf("[%ld] p->status: %s\n", i, p->status > 0 ? "allocated" : "free");
    printf("[%ld] p->prev: %p\n", i, (void*)p->prev);
    printf("[%ld] p->next: %p\n", i, (void*)p->next);
    printf("___________________________\n");
    ++i;
    p = p->next;
  }
  printf("===========================\n");
}