#ifndef _SIMSYS_H_
#define _SIMSYS_H_

/**
 * I pledge my honor that I have abided by the Stevens Honor System.
 * Christopher Hittner
 * James Romph
 */

#include "dirtree.h"

/**
 * Initializes the filesystem. SHOULD ONLY BE CALLED ONCE!
 */
void init_filesystem(long blks, long size);

/**
 * Nullifies the filesystem (init_filesystem will need to be re-called).
 * EQUIVALENT TO 'sudo rm -rf /' with any relevant flags. Frees any memory
 * associated with the simulated filesystem.
 */
void flush_filesystem();

DirTree getRootNode();
DirTree getWorkDirNode();

/**
 * Sets the directory tree node that is currently the working directory.
 * Commands are run based on the value of the working node.
 */
void setWorkDirNode(DirTree);

/**
 * Returns the size of a block, in bytes.
 */
long blockSize();

/* The total number of blocks available on the system */
long numBlocks();
long numSectors();

/**
 * Frees a given block of memory.
 *
 * n - The block to free
 */
void freeBlock(long n);

/**
 * Allocates a single block of memory.
 * 
 * return - The block number allocated, or -1 if an error.
 */
long allocBlock();

/**
 * Determines whether or not there exists enough memory to
 * allocate the number of given bytes.
 *
 * n - The number of blocks requested.
 *
 * return - Whether or not n blocks can be requested.
 */
int enoughMemFor(long n);

/**
 * Get the list of allocated sectors.
 */
LList getAllocData();

long blocksAllocated();
long nextBlock();

/**
 * Gets a relative node in the tree structure.
 * tree - A subtree known to be a child of root.
 * path - A tokenized path between tree and the
 *        destination.
 */
DirTree getRelTree(DirTree, char**);

#endif
