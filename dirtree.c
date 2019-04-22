#include "linkedlist.h"
#include "dirtree.h"
#include "cmds.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct filedata {
    long size;
    LList blocks;
}; typedef struct filedata* FileData;

struct dirdata {
    LList files;
}; typedef struct filedata* DirData;

struct dirtree {
    /* Is the node a file or directory */
    int is_file;

    /* The name of the node */
    char *name;

    /* The parent directory */
    DirTree parent_dir;

    /* Timestamp of when the node was last changed. */
    time_t timestamp;

    union {
        struct filedata file_dta;
        struct dirdata  dir_dta;
    } nodedata;
};


/**
 * Creates a directory node. Duplicates the name w/ strdup().
 */
DirTree makeDirTree(char *name, int is_file) {
    DirTree node = (DirTree) malloc(sizeof(struct dirtree));

    node->name = (char*) malloc((1 + strlen(name)) * sizeof(char));
    strcpy(node->name, name);

    node->is_file = is_file;

    if (is_file) {
        /* Starts as 0 byte file */
        node->nodedata.file_dta.size = 0;
        
        /* Create block list */
        node->nodedata.file_dta.blocks = makeLL();
    } else {
        node->nodedata.dir_dta.files = makeLL();

        node->parent_dir = node;
    }
    
    /* Update the timestamp */
    updateTimestamp(node);

    return node;
}

void flushDirTree(DirTree tree) {
    
    if (tree->is_file) {

        /* Remove and free every value from the block allocation. */
        while (!isEmptyLL(tree->nodedata.file_dta.blocks))
            free(remFromLL(tree->nodedata.file_dta.blocks, 0));
        
        /* Then, free the list */
        free(tree->nodedata.file_dta.blocks);
        tree->nodedata.file_dta.blocks = NULL;
    } else {
        /* Flush every subtree */
        while (!isEmptyLL(tree->nodedata.dir_dta.files)) {
            DirTree subtree = (DirTree) remFromLL(tree->nodedata.dir_dta.files, 0);
            flushDirTree(subtree);
        }

        /* Free and zero */
        free(tree->nodedata.dir_dta.files);
        tree->nodedata.dir_dta.files = NULL;

        tree->parent_dir = NULL;

    }
    
    /* Final frees */
    free(tree->name);
    tree->is_file = 0;
    free(tree);
}

/**
 * Gets the directory node associated with the given path.
 * path - The tokenized path
 *
 * return - The requested node, or NULL if it doesn't exist.
 */
DirTree getDirSubtree(DirTree tree, char *path[]) {
    LLiter iter;

    if (!path || !path[0])
        return tree; /* Found the file */

    else if (tree->is_file)
        return NULL; /* Files do not have subdirectories. */

    else if (!path[0][0] || !strcmp(path[0], "."))
        return getDirSubtree(tree, &path[1]); /* Stay in current dir */

    else if (!strcmp(path[0], ".."))
        return getDirSubtree(tree->parent_dir, &path[1]); /* Go back one directory */
    
    /* Search the subfiles for the next recursive step */
    iter = makeLLiter(tree->nodedata.dir_dta.files);
    while (iterHasNextLL(iter)) {
        
        /* Get the next item from the iterator */
        DirTree child = (DirTree) iterNextLL(iter);

        if (!strcmp(path[0], child->name)) {

            disposeIterLL(iter);
        
            /* The child is should be searched through. */
            return getDirSubtree(child, &path[1]);
        }
    }

    disposeIterLL(iter);

    return NULL;
}

DirTree getTreeParent(DirTree tree) {
    if (tree)
        return tree->parent_dir ? tree->parent_dir : tree;
    else
        return NULL;
}

int addNodeToTree(DirTree tree, char *path[], int is_file) {
    int i;
    DirTree tgtDir;
    char **subpath;
    char *filename;
    
    /* Error check */
    if (!path || !path[0])
        return 3;

    /* Make memory space for subpath */
    i = 0;
    while (path[i]) i++;
    subpath = (char**) malloc(i * sizeof(char*));

    /* Get the filename */
    filename = path[--i];
    
    /* Build the subpath */
    subpath[i--] = NULL;
    for (; i >= 0; i--)
        subpath[i] = path[i];
    
    /* Get the destination */
    tgtDir = getDirSubtree(tree, subpath);
    free(subpath);

    if (tgtDir == NULL) {
        /* Directory not found */
        return 1;
    } else if (tgtDir->is_file) {
        /* Cannot add node to file */
        return 2;
    } else {
        /* Make the file */
        DirTree file = makeDirTree(filename, is_file);

        /* Set the parent directory */
        file->parent_dir = tgtDir;

        /* Add to the file list */
        addToLL(tgtDir->nodedata.dir_dta.files, 0, file);

        /* Update the parent's timestamp to reflect the change. */
        updateTimestamp(tgtDir);
        
        /* No error */
        return 0;
    }

}

int addDirToTree(DirTree tree, char *path[]) {
    return addNodeToTree(tree, path, 0);
}

int addFileToTree(DirTree tree, char *path[]) {
    return addNodeToTree(tree, path, 1);
}

long filesizeOfDirTree(DirTree tree, char *path[]) {
    if (!tree)
        return 0;
    else if (!path || !path[0]) {
        /* Current node is the root. */

        long size = 0; /* Size of the directory node */
        
        /* File check */
        if (tree->is_file)
            return tree->nodedata.file_dta.size;
        
        /* Node is a directory, so it is a combo of sizes. */
        for (int i = sizeOfLL(tree->nodedata.dir_dta.files) - 1; i >= 0; i--)
            size += filesizeOfDirTree((DirTree) getFromLL(tree->nodedata.dir_dta.files, i), NULL);

        return size;
    } else {
        /* Find subtree and get size */
        return filesizeOfDirTree(getDirSubtree(tree, path), NULL);
    }
}

long numFilesInTreeDir(DirTree tree, char *dir[], int rec) {
    if (!tree)
        return 0;
    else if (dir && dir[0]) /* Recursive initial condition */
        return numFilesInTreeDir(getDirSubtree(tree, dir), NULL, rec);
    else {
        int i;
        long count = 0;
        
        /* Check each subfile */
        for (i = sizeOfLL(tree->nodedata.dir_dta.files) - 1; i >= 0; i--) {
            DirTree sub = getFromLL(tree->nodedata.dir_dta.files, i);

            /* Only count directories if recursively checking */
            if (sub->is_file)
                count += sub->nodedata.file_dta.size;
            else
                count += rec ? numFilesInTreeDir(sub, NULL, rec) : 0;
        }

        return count;
    }
}

long treeFileSize(DirTree tree, char *path[]) {
    DirTree file = path ? getDirSubtree(tree, path) : tree;

    if (file->is_file)
        return file->nodedata.file_dta.size;
    else
        return 0;
}


int rmfileFromTree(DirTree tree, char *path[]) {
   if (!tree)
        return 1;
    else if (!path || !path[0]) {
        /* This file is the target */
        DirTree parent = tree->parent_dir;
        
        if (!(tree->is_file)) {
            /* Node is not for a file */
            return 2;
        }
        
        /* Remove the linking from the tree */
        if (parent) {

            /* Update the parent with the change */
            updateTimestamp(parent);

            remFromLL(parent->nodedata.dir_dta.files, indexOfLL(parent->nodedata.dir_dta.files, tree));
            tree->parent_dir = NULL;
        }

        /* Handle the destruction of the file */
        while (sizeOfLL(tree->nodedata.file_dta.blocks))
            remFromLL(tree->nodedata.file_dta.blocks, 0);

        /* Zero the file data */
        tree->nodedata.file_dta.size = 0;

        free(tree->nodedata.file_dta.blocks);
        tree->nodedata.file_dta.blocks = NULL;

        free(tree);

        return 0;

    } else
        return rmfileFromTree(getDirSubtree(tree, path), NULL);
}

int rmdirFromTree(DirTree tree, char *path[]) {
   if (!tree)
        return 1;
    else if (!path || !path[0]) {
        /* This file is the target */
        DirTree parent = tree->parent_dir;
        
        if (tree->is_file) {
            /* Node is a file */
            return 2;
        } else if (sizeOfLL(tree->nodedata.file_dta.blocks)) {
            /* Do not allow a directory with contents to be destroyed */
            return 3;
        }
        
        if (parent) {
            /* Update the parent's timestamp */
            updateTimestamp(parent);

            /* Remove linking with parent */
            remFromLL(parent->nodedata.dir_dta.files, indexOfLL(parent->nodedata.dir_dta.files, tree));
            tree->parent_dir = NULL;
        }

        /* Zero the file data */
        free(tree->nodedata.dir_dta.files);
        tree->nodedata.dir_dta.files = NULL;

        free(tree);

        return 0;

    } else
        return rmfileFromTree(getDirSubtree(tree, path), NULL);
}

int isTreeFile(DirTree tree) {
    return tree->is_file;
}

char* getTreeFilename(DirTree tree) {
    return tree->name;
}

LList getDirTreeChildren(DirTree tree, int alphabetize) {
    LList list = makeLL();
    int i;
    
    if (!tree || tree->is_file)
        return list;
    else if (!alphabetize) /* Simply clone if not adjusting order */
        return cloneLL(tree->nodedata.dir_dta.files);
    
    /* Add each item in alphabetical order */
    for (i = sizeOfLL(tree->nodedata.dir_dta.files) - 1; i >= 0; i--) {
        /* Get an item */
        DirTree elem = (DirTree) getFromLL(tree->nodedata.dir_dta.files, i);

        /* Create a new iterator */
        LLiter iter = makeLLiter(list);
        
        int j;
        /* Iterate until the needed item is found. */
        for (j = 0; iterHasNextLL(iter); j++) {
            DirTree tst = (DirTree) iterNextLL(iter);
            if (strcmp(tst->name, elem->name) < 0) {
                break;
            }
        }

        disposeIterLL(iter);

        addToLL(list, j+1, elem);

    }

    return list;

}

char** pathVecOfTree(DirTree tree) {
    char *name;
    char **vec;

    if (!tree)
        return NULL;
    
    name = tree->name;
    
    if (tree->parent_dir == NULL || tree->parent_dir == tree) {
        /* Tree is root */
        vec = (char**) malloc(2*sizeof(char*));

        vec[0] = (char*) malloc((1 + strlen(name)) * sizeof(char));
        strcpy(vec[0], name);

        vec[1] = NULL;
    } else {
        /* Tree is not root */
        char **par_vec = pathVecOfTree(tree->parent_dir);
        int i;

        for (i = 0; par_vec[i]; i++);
        par_vec[i] = (char*) malloc((1 + strlen(name)) * sizeof(char));
        strcpy(par_vec[i], name);
        
        /* Add to the path */
        vec = realloc(par_vec, (i+2) * sizeof(char*));
        if (!vec) {
            vec = (char**) malloc((i+2) * sizeof(char*));
            memcpy(vec, par_vec, (i+1) * sizeof(char*));
            free_str_vec(par_vec);
        }

        vec[i+1] = NULL;
        
        return vec;


    }

    return vec;

}

time_t getTreeTimestamp(DirTree tree) {
    if (tree)
        return tree->timestamp;
    else
        return time(NULL);
}

LList getTreeFileBlocks(DirTree file) {
    
    if (!file || !(file->is_file))
        return NULL;
    else
        return cloneLL(file->nodedata.file_dta.blocks);

}

void updateFileSize(DirTree tree, long newSize) {
    if (tree && tree->is_file)
        tree->nodedata.file_dta.size = newSize;
}

void updateTimestamp(DirTree tree) {
    if (tree)
        tree->timestamp = time(NULL);
}

void setTimestamp(DirTree tree, time_t t) {
    if (tree)
        tree->timestamp = t;
}

void assignMemoryBlock(DirTree tree, long blk) {
    long *tmp;
    
    /* Edge case checks */
    if (!tree)
        return;
    else if (!(tree->is_file))
        return;
    
    tmp = (long*) malloc(sizeof(long));
    *tmp = blk;

    addToLL(tree->nodedata.file_dta.blocks, 0, tmp);
}

long releaseMemoryBlock(DirTree tree) {
    long *res;
    long val;

    if (!tree || !(tree->is_file))
        return -1;

    res = (long*) remFromLL(tree->nodedata.file_dta.blocks, 0);
    val = *res;
    free(res);

    return val;
}



