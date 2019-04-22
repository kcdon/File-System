#include "simsys.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

long BLOCK_SIZE = 0;
long NUM_BLOCKS = 0;

DirTree ROOT_DIR = NULL;
DirTree WORK_DIR = NULL;

/**
 * The list of allocated blocks.
 * Invariant - For any index i = 2n,
 *             i is the start of a memory
 *             block, and i+1 is the end of
 *             the same block.
 * Logic: In memory, we suppose that we have some set
 *        of allocated sectors B_i = [a, b), where all
 *        addresses in blocks [a, b) are taken. We say
 *        that we will hold a list of these pairs, such
 *        that any two consecutive values represent either
 *        the beginning and end of sectors or the borders
 *        of consective sectors.
 */
LList MEM_ALLOC;

void init_filesystem(long blk_size, long size) {
    
    /* Prevent the function from being called more than once. */
    if (ROOT_DIR) {
        printf("\033[1m\033[31mERROR\033[0m: Cannot call init_filesystem more than once!\n");
        return;
    }

    /* The size of a block and the number of blocks are stored. */
    BLOCK_SIZE = blk_size;
    NUM_BLOCKS = size / blk_size;
    
    /* The root node of the filesystem. */
    ROOT_DIR = makeDirTree("", 0);
    
    /* The initial working directory is root by default. */
    WORK_DIR = ROOT_DIR;
    
    /* The list of memory allocations. */
    MEM_ALLOC = makeLL();
}

void flush_filesystem() {
    
    WORK_DIR = NULL;

    /* Recursively destroy the file tree */
    flushDirTree(ROOT_DIR);
    
    /* Dispose of memory allocation */
    while(!isEmptyLL(MEM_ALLOC))
        free(remFromLL(MEM_ALLOC, 0));
    free(MEM_ALLOC);
    MEM_ALLOC = NULL;

    BLOCK_SIZE = 0;
    NUM_BLOCKS = 0;

}

DirTree getRootNode() {
    return ROOT_DIR;
}

DirTree getWorkDirNode() {
    return WORK_DIR;
}

void setWorkDirNode(DirTree node) {
    WORK_DIR = node;
}

long blockSize() {
    return BLOCK_SIZE;
}

long numBlocks() {
    return NUM_BLOCKS;
}

long numSectors() {
    return sizeOfLL(MEM_ALLOC) / 2;
}

void freeBlock(long blk) {
    int sectors = sizeOfLL(MEM_ALLOC);
    long lo, hi;
    int i;
    
    /* End result: The ith block contains block blk. */
    for (i = 0; i < sectors; i++) {
        /* If block i contains blk, it must be freed from the block. */
        if (   *((long*) getFromLL(MEM_ALLOC, 2*i)) <= blk
            && *((long*) getFromLL(MEM_ALLOC, 2*i+1)) > blk)
                break;
    }

    if (i == sectors)
        return;
    
    lo = *((long*) getFromLL(MEM_ALLOC, 2*i));
    hi = *((long*) getFromLL(MEM_ALLOC, 2*i+1));
    
    if (lo == hi-1) {
        /* The sector is of size 1, so free the whole sector */
        free(remFromLL(MEM_ALLOC, 2*i));
        free(remFromLL(MEM_ALLOC, 2*i));
    } else if (lo == blk) {
        /* The block is at the front of the sector */
        *((long*) getFromLL(MEM_ALLOC, 2*i)) += 1;
    } else if (hi-1 == blk) {
        /* The block is at the back of the sector */
        *((long*) getFromLL(MEM_ALLOC, 2*i+1)) -= 1;
    } else {
        /* IN any other case, the free will split the block in two. */

        /* The new bounds are computed (mLo and mHi represent the freed block) */
        long *mLo = (long*) malloc(sizeof(long));
        long *mHi = (long*) malloc(sizeof(long));

        *mLo = blk;
        *mHi = blk+1;
        
        /* Create a one block gap in the memory by inserting the pair */
        /* [a,b)...[c,d)...[e,f)... ==> [a,b)...[c,mLo)x[mHi, d)... */
        addToLL(MEM_ALLOC, 2*i+1, mLo);
        addToLL(MEM_ALLOC, 2*i+2, mHi);
    }

}

long allocBlock() {
    
    /* The number of sectors */
    long sectors = sizeOfLL(MEM_ALLOC) / 2;
    long *tmp;

    if (sectors == 0) {
        /* No memory allocated */

        /* We add the two bounds of the block to allocate. */
        tmp = (long*) malloc(sizeof(long));
        *tmp = 0;
        addToLL(MEM_ALLOC, 0, tmp);
        
        tmp = (long*) malloc(sizeof(long));
        *tmp = 1;
        addToLL(MEM_ALLOC, 1, tmp);

        return 0;
        
    } else {
        /* The bounds of the memory available */
        long min_free = 0;
        long max_free = *((long*) getFromLL(MEM_ALLOC, 0));
        
        if (min_free == max_free) {
            /* Can't place in front, 0 bytes at the front */

            /* The new minimum is the end of the first alloc'd sector */
            min_free = *((long*) getFromLL(MEM_ALLOC, 1));

            /* The end of the free sector is either the start of the next
               sector or the end of memory */
            max_free = sectors == 1
                                  ? NUM_BLOCKS
                                  : *((long*) getFromLL(MEM_ALLOC, 2));
            
            if (min_free == max_free) {
                /* No available memory */
                return -1;
            } else if (min_free == max_free - 1) {
                /* Closes a one-byte gap in memory (only one byte between) */
                free(remFromLL(MEM_ALLOC, 1));
                free(remFromLL(MEM_ALLOC, 1));
            } else {
                *((long*) getFromLL(MEM_ALLOC, 1)) += 1;
            }
            
            /* The first free block was alloc'd */
            return min_free;

        } else if (min_free == max_free - 1) {
            /* Close a one-block frag in the front */
            *((long*) getFromLL(MEM_ALLOC, 0)) = 0;

            return 0;
        } else {
            /* Memory in non-bordering space */
            tmp = (long*) malloc(sizeof(long));
            *tmp = 0;
            addToLL(MEM_ALLOC, 0, tmp);
            
            tmp = (long*) malloc(sizeof(long));
            *tmp = 1;
            addToLL(MEM_ALLOC, 1, tmp);

            return 0;
        }


    }

}

int enoughMemFor(long amt) {
    int secs = sizeOfLL(MEM_ALLOC) / 2;
    long avail = 0;
    int i;

    /* If memory is empty, the answer is simple. */
    if (secs == 0)
        return NUM_BLOCKS >= amt;
    
    /* The amount of memory is equal to the total amount
       of memory between each block of allocated memory. */
    avail = *((long*) getFromLL(MEM_ALLOC, 0));
    for (i = 1; i < secs && avail < amt; i++) {
        avail += *((long*) getFromLL(MEM_ALLOC, 2*i-1))
                 - *((long*) getFromLL(MEM_ALLOC, 2*i));
    }
    
    /* If all gaps were viewed, tack the end onto the total */
    if (i == secs)
        avail += NUM_BLOCKS - *((long*) getFromLL(MEM_ALLOC, 2*secs-1));
    
    /* Answer the question */
    return amt <= avail;

}

LList getAllocData() {
    return cloneLL(MEM_ALLOC);
}

long blocksAllocated() {
    long amt = 0;

    LLiter iter = makeLLiter(MEM_ALLOC);
    while (iterHasNextLL(iter))
        amt -= (*((long*) iterNextLL(iter)) - *((long*) iterNextLL(iter)));
    
    return amt;
}

long nextBlock() {
    if (isEmptyLL(MEM_ALLOC))
        return 0;
    else if (*((long*) getFromLL(MEM_ALLOC, 0)))
        return 0;
    else
        return *((long*) getFromLL(MEM_ALLOC, 1));
}

DirTree getRelTree(DirTree tree, char **path) {
    if (!path)
        return getRootNode();
    else if (!path[0])
        return getWorkDirNode();
    else if (strcmp(path[0], ""))
        return getDirSubtree(tree, path);
    else
        return getDirSubtree(getRootNode(), &path[1]);
}





