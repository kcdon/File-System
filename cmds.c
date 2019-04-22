#include "cmds.h"
#include "dirtree.h"
#include "simsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** str_to_vec(char *str, char split_c) {
    char** vec = (char**) malloc(sizeof(char*));
    char *next;
    int i, j, k;
    int size = 0;

    vec[0] = NULL;
    
    k = 0;
    while (1) {
        int m, n;
        char **tmp;
        
        for (m = 0, n = 0; str[k+n+m] != split_c && str[k+n+m]; n++) {
            /* Skip backslashes */
            if (str[k+n+m] == '\\'/* && str[k+n+m+1]*/)
                m++;
        }
        
        /* Make the new string */
        next = (char*) malloc((n+1) * sizeof(char));
        for (i = 0, j = 0; i < n; i++) {
            if (str[k+i+j] == '\\')
                j++;
            next[i] = str[k+i+j];
        }
        next[n] = '\0';

        vec[size] = next;

        /* Realloc */
        size++;
        tmp = (char**) realloc(vec, (size+1) * sizeof(char*));
        if (!tmp) {
            printf("Have to reallocate.\n");
            tmp = (char**) malloc((size+1) * sizeof(char*));
            memcpy(tmp, vec, size * sizeof(char*));
        }

        vec = tmp;
        vec[size] = NULL;
        
        /* Update k */
        while (str[k+m+n] == split_c)
            k++;

        if (str[k+m+n]) 
            k += n+m;
        else
            return vec;

    }

}

void free_str_vec(char **vec) {
    int i;
    for (i = 0; vec[i]; i++) {
        free(vec[i]);
        vec[i] = NULL;
    }
    free(vec);
}

void error_message(char *cmd, char *mssg) {
    printf("%s: Cannot perform operation: %s\n", cmd, mssg);
}

void printTreeNode(DirTree node, int fullpath, int details) {
    char *filename;
    char buff[32];

    int is_file;
    long filesize;

    if (!node)
        return;

    is_file = isTreeFile(node);
    filesize = treeFileSize(node, NULL);
    filename = getTreeFilename(node);
    
    if (details) {
        time_t now = time(NULL);
        struct tm* timestamp;
        int currYear;

        time_t time = getTreeTimestamp(node);
        
        timestamp = localtime(&now);
        currYear = timestamp->tm_year;

        timestamp = localtime(&time);
        
        if (currYear == timestamp->tm_year)
            strftime(buff, 32*sizeof(char), "%b %d %H:%M", timestamp);
        else
            strftime(buff, 32*sizeof(char), "%b %d %Y", timestamp);

        printf("%-15s ", buff);
        printf("%-15ld ", filesize);

    }
    
    /* Blue bold for the files */
    if (!is_file)
        printf("\033[1m\033[34m");

    if (fullpath) {
        /* Show full path */
        char **path = pathVecOfTree(node);
        int i;
        
        /* Print each file layer */
        for (i = 0; path[i]; i++)
            printf("%s%s", path[i],
                path[i+1] ? "/" : ""
        );

        free_str_vec(path);

    } else
        printf("%s", filename);
    
    if (!is_file)
            printf("/");

    printf("\033[0m\n");
 
            /*
            int is_file = isTreeFile(file);
             
            printf("%s%s%s", 
                is_file ? "" : "\033[1m\033[34m",
                getTreeFilename(file),
                "\033[0m");
            
            *//* Prints only if the child is a file *//*
            if (is_file)
                printf(" - %ldB",treeFileSize(file, NULL));

            printf("\n");
            */   
}

/**
 * Runs a command.
 *
 * argv - The command vector to execute.
 */
void cmd_exec(char *argv[]) {
    SimCmd cmd;
    char *name = argv[0];

    /* Don't need to bother with empty line */
    if (!argv || !argv[0])
        return;
    
    if (!strcmp(name, "cd"))
        cmd = cmd_cd;
    else if (!strcmp(name, "ls"))
        cmd = cmd_ls;
    else if (!strcmp(name, "mkdir"))
        cmd = cmd_mkdir;
    else if (!strcmp(name, "create"))
        cmd = cmd_create;
    else if (!strcmp(name, "append"))
        cmd = cmd_append;
    else if (!strcmp(name, "remove"))
        cmd = cmd_remove;
    else if (!strcmp(name, "delete"))
        cmd = cmd_delete;
    else if (!strcmp(name, "exit"))
        cmd = cmd_exit;
    else if (!strcmp(name, "dir"))
        cmd = cmd_dir;
    else if (!strcmp(name, "prfiles"))
        cmd = cmd_prfiles;
    else if (!strcmp(name, "cd..")) {
        char *args[3];
        args[0] = "cd";
        args[1] = "..";
        args[2] = NULL;
        cmd_exec(args);
        return;
    } else {
        printf("%s: command not found.\n", argv[0]);
        return;
    }

    cmd(argv);
}

int cmd_cd(char *argv[]) {
    char **dirtoks;
    int i = 1;
    
    if (!argv[1]) {
        /* Default is to go to root. */
        setWorkDirNode(getRootNode());
        return 0;
    } else while (argv[i]) {
        DirTree tgt;

        /* Build a token vector */
        dirtoks = str_to_vec(argv[i], '/');

        /* Get the destination */
        tgt = getRelTree(getWorkDirNode(), dirtoks);

        /* Free the vector */
        free_str_vec(dirtoks);
        
        if (!tgt) {
            printf("cd: %s: No such file or directory\n", argv[i]);
            return 1;
        } else if (isTreeFile(tgt)) {
            printf("cd: %s: Target is not a directory\n", argv[i]);
            return 1;
        } else {
            setWorkDirNode(tgt);
        }

        i++;
    }

    return 0;
}

/**
 * Lists the contents of a directory.
 */
int cmd_ls(char *argv[]) {
    DirTree tgt;
    DirTree working_dir = getWorkDirNode();

    if (argv[1]) {
        char **dirtoks = str_to_vec(argv[1], '/');

        /* Adjust the node to print */
        tgt = getRelTree(working_dir, dirtoks);

        free_str_vec(dirtoks);
    } else
        tgt = working_dir;

    if (!tgt) {
        printf("ls: cannot access '%s': No such file or directory\n", argv[1]);
        return 1;
    } else if (isTreeFile(tgt)) {
        printf("ls: cannot access '%s': Target is not a directory.\n", argv[1]);
        return 1;
    } else {
        LList files = getDirTreeChildren(tgt, 1);

        printf("total %i\n", sizeOfLL(files));
        
        /* Go through each file, removing from the list as it goes */
        while (!isEmptyLL(files)) {
            DirTree file = (DirTree) remFromLL(files, 0);

            printTreeNode(file, 0, 1);
        }
        
        /* Delete the list */
        free(files);
    }
    
    return 0;
}

/**
 * Creates a directory, or set of directories.
 */
int cmd_mkdir(char *argv[]) {
    
    if (!argv[1]) {
        printf("mkdir: missing operand");
        return 1;
    } else {
        /* Get a list of all of the files in the directory */
        int errCode = 0;

        int i = 1;
        while (argv[i]) {
            /* Build a path */
            char **path = str_to_vec(argv[i], '/');
            char *dirnm;
            int j, k;
            DirTree tgtDir;
            LList files;

            /* Check whether or not the file already exists */
            if (getRelTree(getWorkDirNode(), path)) {
                printf("mkdir: Cannot create directory '%s': Already exists\n", argv[i]);
                errCode = 1;
                i++;
                continue;
            }
            
            /* Separate the target name and its path */
            for (k = 0; path[k]; k++);
            dirnm = path[k-1];
            path[k-1] = NULL;
            
            /* Get the potential parent node */
            tgtDir = getRelTree(getWorkDirNode(), path);

            if (!tgtDir) {
                /* The containing path does not exist. */
                printf("mkdir: cannot create directory '%s': No such file or directory\n", argv[i]);
                errCode = 1;
                i++;
                continue;
            }

            files = getDirTreeChildren(tgtDir, 0);

            for (j = sizeOfLL(files) - 1; j >= 0; j--) {
                DirTree file = (DirTree) getFromLL(files, j);

                if (!strcmp(getTreeFilename(file), argv[i])) {
                    /* Don't make duplicate directories */
                    printf("mkdir: cannot create directory '%s': Already exists\n", argv[i]);
                    errCode = 1;
                    break;
                }
            }

            path[k-1] = dirnm;
            
            if (j < 0) {
                /* Add the directory */
                addDirToTree(getWorkDirNode(), path);
            }

            /* Free used memory */
            free_str_vec(path);
            while (!isEmptyLL(files))
                remFromLL(files, 0);
            free(files);

            i++;
        }

        
        return errCode;
    }

}

/**
 * Creates a file in the file structure.
 */
int cmd_create(char *argv[]) {
    if (!argv[1]) {
        error_message("create", "No file names provided.");
        return 1;
    } else {
        /* Get a list of all of the files in the directory */
        int errCode = 0;

        int i = 1;
        while (argv[i]) {
            /* Build a path */
            char **path = str_to_vec(argv[i], '/');
            char *filenm;
            int j, k;
            DirTree tgtDir;
            LList files;

            /* Check whether or not the file already exists */
            if (getRelTree(getWorkDirNode(), path)) {
                printf("mkdir: cannot make file '%s': Already exists\n", argv[i]);
                errCode = 1;
                i++;
                continue;
            }
            

            
            /* Separate the target name and its path */
            for (k = 0; path[k]; k++);
            filenm = path[k-1];
            path[k-1] = NULL;
            
            /* Get the potential parent node */
            tgtDir = getRelTree(getWorkDirNode(), path);

            /* Check to see if it exists */
            if (!tgtDir) {
                printf("mkdir: cannot create file '%s': No such file or directory\n", argv[i]);
                errCode = 1;
                i++;
                continue;
            }

            files = getDirTreeChildren(tgtDir, 0);


            for (j = sizeOfLL(files) - 1; j >= 0; j--) {
                DirTree file = (DirTree) getFromLL(files, j);

                if (!strcmp(getTreeFilename(file), argv[i])) {
                    /* Don't make duplicate directories */
                    printf("create: cannot create file %s: Already exists.\n", argv[i]);
                    errCode = 1;
                    break;
                }
            }

            path[k-1] = filenm;
            
            if (j < 0) {
                /* Add the directory */
                addFileToTree(getWorkDirNode(), path);
            }

            /* Free used memory */
            free_str_vec(path);
            while (!isEmptyLL(files))
                remFromLL(files, 0);
            free(files);

            i++;
        }

        
        return errCode;
    }
}

int cmd_append(char *argv[]) {
    if (!argv[1] || !argv[2]) {
        printf("append: missing operand\n");
        return 1;
    } else {
        int errCode = 0;
        
        char **path = str_to_vec(argv[1], '/');
        DirTree tgt = getRelTree(getWorkDirNode(), path);
        long request = atol(argv[2]);
        
        if (!tgt) {
            errCode = 1;
            printf("append: cannot modify '%s': No such file\n", argv[1]);
        } else if (request <= 0) {
            errCode = 1;
            printf("append: cannot append nonpositive memory\n");
        } else if (isTreeFile(tgt)) {

            /* Calculate the necessary block allocation needed */
            long fileSizeBefore;
            long fileSizeAfter;
            long blocksNeeded;

            fileSizeBefore = treeFileSize(tgt, NULL);
            fileSizeAfter = fileSizeBefore + request;

            /* Difference of ceiling divisions for old/new block size */
            blocksNeeded = fileSizeBefore 
                            ? ((fileSizeAfter - 1) / blockSize() - (fileSizeBefore - 1) / blockSize())
                            : (1 + (fileSizeAfter - 1) / blockSize());

            /* Update the file */
            if (enoughMemFor(blocksNeeded)) {

                printf("Allocating %ld bytes (needs %ld blocks)...\n", request, blocksNeeded);

                /* Allocate blocks */
                while (blocksNeeded > 0) {
                    assignMemoryBlock(tgt, allocBlock());
                    blocksNeeded--;
                }

                updateFileSize(tgt, fileSizeAfter);

                updateTimestamp(tgt);
                updateTimestamp(getTreeParent(tgt));

            } else {
                errCode = 1;
                printf("append: cannot modify '%s': Insufficient memory space to allocate %ld blocks\n", argv[1], blocksNeeded);
            }

        } else {
            errCode = 1;
            printf("append: cannot modify '%s': Not a file\n", argv[1]);
        }


        return errCode;
    }
}

int cmd_remove(char *argv[]) {
    if (!argv[1] || !argv[2]) {
        printf("remove: missing operand\n");
        return 1;
    } else {
        int errCode = 0;
        
        char **path = str_to_vec(argv[1], '/');
        DirTree tgt = getRelTree(getWorkDirNode(), path);

        long request = atol(argv[2]);
        
        if (!tgt) {
            errCode = 1;
            printf("remove: cannot modify '%s': No such file\n", argv[1]);
        } else if (request <= 0) {
            errCode = 1;
            printf("remove: cannot remove nonpositive memory\n");
        } else if (isTreeFile(tgt)) {

            /* Calculate the necessary block deallocation needed */
            long fileSizeBefore;
            long fileSizeAfter;
            long blocksNeeded;

            fileSizeBefore = treeFileSize(tgt, NULL);
            fileSizeAfter = fileSizeBefore - request;

            /* Difference of ceiling divisions for old/new block size */
            blocksNeeded = fileSizeAfter
                            ? ((fileSizeBefore - 1) / blockSize() - (fileSizeAfter - 1) / blockSize())
                            : (1 + (fileSizeBefore - 1) / blockSize());

            /* Update the file */
            if (fileSizeAfter < 0) {
                errCode = 1;
                printf("remove: cannot modify '%s': More blocks requested for deletion than exist\n", argv[1]);
            } else {
                printf("Deallocating %ld bytes (revoking %ld blocks)...\n", request, blocksNeeded);
                
                /* Deallocate the blocks */
                while (blocksNeeded > 0) {
                    freeBlock(releaseMemoryBlock(tgt));
                    blocksNeeded--;
                }

                updateFileSize(tgt, fileSizeAfter);
                updateTimestamp(tgt);
                updateTimestamp(getTreeParent(tgt));
            }

        } else {
            errCode = 1;
            printf("remove: cannot modify '%s': Not a file\n", argv[1]);
        }

        return errCode;
    }
}


int cmd_delete(char *argv[]) {
    if (!argv[1]) {
        printf("rm: missing operand\n");
        return 1;
    } else {
        int errCode = 0;
        int i = 1;
        
        while (argv[i]) {
            char **path = str_to_vec(argv[i], '/');
            DirTree tgt = getRelTree(getWorkDirNode(), path);
            
            if (tgt == getWorkDirNode()) {
                /* Don't delete directory that is currently in use. */
                errCode = 1;
                printf("delete: failed to remove '%s': Directory currently in use\n", argv[i]);
            } else if (!tgt) {
                errCode = 1;
                printf("delete: cannot delete '%s': No such file or directory\n", argv[i]);
            } else if (isTreeFile(tgt)) {
                /* The currently allocated memory blocks */
                LList blocks = getTreeFileBlocks(tgt);
                
                /* Free each used memory block one by one */
                while (!isEmptyLL(blocks)) {
                    long *blk = (long*) remFromLL(blocks, 0);
                    freeBlock(*blk);
                    free(blk);
                }
                free(blocks);
                
                /* Remove the file */
                errCode |= rmfileFromTree(tgt, NULL);

            } else {
                /* Handle directory removal. */
                LList children = getDirTreeChildren(tgt, 0);
                
                /* Allow deletion if the directory is empty */
                if (isEmptyLL(children))
                    errCode |= rmdirFromTree(tgt, NULL);
                else {
                    errCode = 1;
                    printf("delete: failed to remove '%s': Directory not empty\n", argv[i]);
                }
                
                /* Free the child list */
                while(!isEmptyLL(children))
                    remFromLL(children, 0);
                free(children);
            }

            i++;

        }

        return errCode;
    }
}


/**
 * Terminates the program.
 */
int cmd_exit(char *argv[]) {
    
    /* Cleans the filesystem. */
    flush_filesystem();
    
    if (argv[1])
        exit(atoi(argv[1]));
    else
        exit(0);

    return 1;

}

int cmd_dir(char *argv[]) {
    
    DirTree root;
    LList bfs_list = makeLL();
    
    /* Get the top directory of the BFS */
    if (!argv[1])
        root = getWorkDirNode();
    else {
        char **dirtoks = str_to_vec(argv[1], '/');
        root = getRelTree(getWorkDirNode(), dirtoks);
        free_str_vec(dirtoks);
    }
    
    /* Initial value is the root */
    appendToLL(bfs_list, root);
    
    while (!isEmptyLL(bfs_list)) {
        DirTree curr = (DirTree) remFromLL(bfs_list, 0);
        
        /* Print the front file */
        printTreeNode(curr, 1, 0);
        
        if (!isTreeFile(curr)) {
            /* Is a directory; add all children */
            LList children = getDirTreeChildren(curr, 1);

            while (!isEmptyLL(children))
                appendToLL(bfs_list, remFromLL(children, 0));

            free(children);

        }

    }

    free(bfs_list);

    return 0;
    
}

/* Used for nlogn sort of longs */
void mergesort_longs(long *list, int lo, int hi) {
    if (hi - lo == 2) {
        /* Pair case */
        if (list[lo] > list[lo+1]) {
            long tmp = list[lo];
            list[lo] = list[lo+1];
            list[lo+1] = tmp;
        }
    } else if (hi - lo > 2) {
        long *tmp;

        int c = (lo + hi) / 2;

        int i = lo;
        int j = c;
        int k = 0;

        mergesort_longs(list, lo, c);
        mergesort_longs(list, c, hi);
        
        tmp = (long*) malloc((hi - lo) * sizeof(long));
        
        /* Build sorted into the temporary array */
        while (i < c && j < hi) {
            if (list[i] < list[j])
                tmp[k++] = list[i++];
            else
                tmp[k++] = list[j++];
        }
        
        while (j < hi)
            tmp[k++] = list[j++];

        while (i < c)
            tmp[k++] = list[i++];

        /* Reinsert */
        while (k--)
            list[lo+k] = tmp[k];

        free(tmp);

    }

}

int cmd_prfiles(char *argv[]) {
    DirTree root;
    LList bfs_list = makeLL();
    
    /* Get the top directory of the BFS */
    if (!argv[1])
        root = getWorkDirNode();
    else {
        char **dirtoks = str_to_vec(argv[1], '/');
        root = getRelTree(getWorkDirNode(), dirtoks);
        free_str_vec(dirtoks);
    }
    
    appendToLL(bfs_list, root);

    while (!isEmptyLL(bfs_list)) {
        DirTree curr = (DirTree) remFromLL(bfs_list, 0);
                        
        /* Breadth-first recursive definition */
        if (!isTreeFile(curr)) {
            /* Is a directory; add all children */
            LList children = getDirTreeChildren(curr, 1);

            while (!isEmptyLL(children))
                appendToLL(bfs_list, remFromLL(children, 0));

            free(children);

        } else {
            /* Get block information */
            LList blocks = getTreeFileBlocks(curr);
            int num_blks = sizeOfLL(blocks);

            /* Used as a scratch space for printing. */
            long *blks = (long*) malloc(num_blks * sizeof(long));
            int i;
            
            /* For printing */
            int contig = 0;

            /* Iterator */
            LLiter iter;
            
            /* Print basic file data */
            printTreeNode(curr, 1, 1);
            
            /* Dequeue each block number */
            iter = makeLLiter(blocks);
            for (i = 0; iterHasNextLL(iter); i++)
                blks[i] = *((long*) iterNextLL(iter));

            free(blocks);
            disposeIterLL(iter);

            printf("%i blocks%s", num_blks, num_blks ? ": " : "");
            
            /* Sort the blocks */
            mergesort_longs(blks, 0, num_blks);

            for (i = 0; i < num_blks; i++) {

                /* Format print the blocks. */
                if (i > 0 && blks[i] - 1 == blks[i-1]) {
                    if (!contig) {
                        printf("-");
                        contig = 1;
                    } 
                } else {
                    if (contig) {
                        printf("%ld %ld", blks[i-1], blks[i]);
                        contig = 0;
                    } else
                        printf(" %ld", blks[i]);
                }
            }
            
            /* Print the last value if necessary */
            if (contig)
                printf("%ld", blks[i-1]);

            printf("\n\n");

            /* Free the array */
            free(blks);


        }

    }
    
    /* Dispose of the list */
    free(bfs_list);

    return 0;

}




