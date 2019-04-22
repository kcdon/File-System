#ifndef _DIRTREE_H_
#define _DIRTREE_H_

/**
 * I pledge my honor that I have abided by the Stevens Honor System.
 * Christopher Hittner
 * James Romph
 */

#include "linkedlist.h"

#include <time.h>

struct dirtree;
typedef struct dirtree* DirTree;

long BLOCK_SIZE;

/**
 * Creates a directory tree that is either an extendable
 * node (directory) or a leaf node (file).
 */
DirTree makeDirTree(char *name, int is_file);

/**
 * Disposes of a tree and any subtrees it has. The given
 * tree and any affected data will be freed.
 */
void flushDirTree(DirTree tree);

/**
 * Gets the subtree, supertree or relative tree of the given tree
 * found by following the given path.
 */
DirTree getDirSubtree(DirTree tree, char *path[]);

DirTree getTreeParent(DirTree);

/**
 * Adds a directory path to the tree
 * path - The filepath, vectorized
 *
 * return - An nonzero error code if something went wrong:
 *          1 - File not found
 *          2 - Tried to add file to a file
 *          3 - Bad argument(s)
 */
int addDirToTree(DirTree tree, char *path[]);

/**
 * Adds a file to the system.
 * path - The filepath, ending in the filename
 *
 * return - An nonzero error code if something went wrong:
 *          1 - File not found.
 *          2 - Tried to add file to a file
 *          3 - Bad argument(s)
 */
int addFileToTree(DirTree tree, char *path[]);

/**
 * Removes a file from the system.
 * path - The filepath, ending in the filename.
 *
 * return - A nonzero error code if something went wrong:
 *          1 - File not found.
 *          2 - Tried to remove directory when file expected.
 */
int rmfileFromTree(DirTree tree, char *path[]);

/**
 * Removes a directory from the system.
 * path - The filepath, ending in the filename.
 *
 * return - A nonzero error code if something went wrong:
 *          1 - File not found.
 *          2 - Tried to remove file when directory expected.
 */
int rmdirFromTree(DirTree tree, char *path[]);

/* Statistical functions */

/**
 * Computes the size of a directory, in bytes.
 */
long filesizeOfDirTree(DirTree tree, char *path[]);

/**
 * Computes the number of files in the directory.
 */
long numFilesInTreeDir(DirTree tree, char *dir[], int recursive);

/**
 * Computes size of file, in bytes. If actually a directory, returns 0.
 */
long treeFileSize(DirTree tree, char *filepath[]);

/**
 * Whether or not a given tree root is a directory
 */
int isTreeFile(DirTree tree);

/**
 * The children of the given tree root.
 */
LList getDirTreeChildren(DirTree tree, int alphabetize);

/**
 * Generates the path vector of a given directory or file node.
 */
char** pathVecOfTree(DirTree tree);

/**
 * Gets and returns the name of a given DirTree node.
 */
char* getTreeFilename(DirTree tree);

/**
 * Gets the last access time of the given node.
 */
time_t getTreeTimestamp(DirTree);

/**
 * Retrieves a copy of the block list for a given file node.
 *
 * file - A DirTree that is known to be a file.
 *
 * return - A clone of the block list.
 */
LList getTreeFileBlocks(DirTree file);

/**
 * Updates the file with a new size. Should be called whenever
 * the size of the file changes.
 */
void updateFileSize(DirTree, long);

/**
 * Updates timestamp of a tree node. Should be called whenever
 * a file is modified.
 */
void updateTimestamp(DirTree);

/**
 * Manually updates timestamp of a tree node.
 */
void setTimestamp(DirTree, time_t);

/**
 * Assigns a block of memory to a file.
 * precondition - Block b has already been allocated.
 *
 * file - A DirTree node that corresponds to a file that shoud
 *        receive the given block for use.
 * b    - The id of an allocated block.
 */
void assignMemoryBlock(DirTree file, long b);

/**
 * Revokes a block of memory from a file. Assumes that the user
 * will follow up by freeing the provided block id.
 *
 * file - A DirTree corresponding to a file.
 * 
 * return - The id of a block to be freed. The block will be an
 *          allocated block unless it was somehow freed.
 */
long releaseMemoryBlock(DirTree file);

#endif
