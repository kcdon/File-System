#include "dirtree.h"
#include "cmds.h"
#include "simsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

void testLinkedList() {
    int i;

    int arr[] = { 0, 1, 2 };

    LList testLL = makeLL();

    appendToLL(testLL, &arr[0]);
    addToLL(testLL, 7, &arr[1]);
    addToLL(testLL, 2, &arr[2]);

    for (i = 0; i < 3; i++) {
        printf("Here's a number:\n");
        printf("The number %i\n", *((int*) getFromLL(testLL, i)));
    }
    printf("\n");
}

void testTokenize() {
    
    char **vec;
    int i;
    
    printf("Here is four tokens:\n");
    vec = str_to_vec("Here is four tokens.", ' ');
    for (i = 0; vec[i]; i++)
        printf("Token %i: %s\n", i, vec[i]);

    printf("Will free...\n");
    free_str_vec(vec);
    

    printf("\nHere's the tokens for compiling a C program:\n");
    vec = str_to_vec("gcc -o exec *.c -Wall -Werror --pedantic", ' ');
    for (i = 0; vec[i]; i++)
        printf("Token %i: %s\n", i, vec[i]);

    printf("Will free...\n");
    free_str_vec(vec);


    printf("\nHere's '/home/chrishittner tokenized:\n");
    vec = str_to_vec("/home/chrishittner", '/');
    for (i = 0; vec[i]; i++)
        printf("Token %i: %s\n", i, vec[i]);

    printf("Will free...\n");
    free_str_vec(vec);


    printf("\nHere's a single token with spaces in it:\n");
    vec = str_to_vec("This\\ is\\ one\\ token", ' ');
    for (i = 0; vec[i]; i++)
        printf("Token %i: %s\n", i, vec[i]);

    printf("Will free...\n");
    free_str_vec(vec);

    printf("\nstr_to_vec() and free_str_vec() test complete.\n\n");
}

void testDirTree() {

    printf("Making a directory tree\n");

    DirTree root = makeDirTree("", 0);
    char *path[16];
    int i;
    
    path[1] = NULL;
 
    printf("Adding /etc/\n");
    path[0] = "etc";
    addDirToTree(root, path);
    
    printf("Adding /usr/\n");
    path[0] = "usr";
    addDirToTree(root, path);
   
    printf("Adding /bin/\n");
    path[0] = "bin";
    addDirToTree(root, path);

    printf("Retrieving file list\n");
    LList files = getDirTreeChildren(root, 1);

    printf("Files in root:\n");
    for (i = 0; i < 3; i++) {
        DirTree file = (DirTree) getFromLL(files, i);
        printf("%s ", getTreeFilename(file));
    }
    printf("\n\n");

    printf("Testing list removal:\n");
    for (i = 0; i < 3; i++) {
        DirTree file = (DirTree) remFromLL(files, 0);
        printf("%s ", getTreeFilename(file));
    }

    printf("\n\nDirectory tree test complete.");

}

void testCmds() {
    char *path[16];
    int j;
    long l;
    
    init_filesystem(32, 512);

    printf("Running 'mkdir etc usr bin' (should work)\n");
    path[0] = "mkdir";
    path[1] = "etc";
    path[2] = "usr";
    path[3] = "bin";
    path[4] = NULL;
    cmd_mkdir(path);

    printf("\nRunning 'mkdir usr/bin etc lib' (should reject etc)\n");
    path[1] = "usr/bin";
    path[2] = "etc";
    path[3] = "lib";
    cmd_mkdir(path);

    printf("\nRunning 'ls'\n");
    path[0] = "ls";
    path[1] = NULL;
    cmd_ls(path);

    printf("\nRunning 'ls usr'\n");
    path[1] = "usr";
    path[2] = NULL;
    cmd_ls(path);

    printf("\nRunning 'ls home/chittner' (should fail)\n");
    path[1] = "home/chittner";
    cmd_ls(path);
    
    /* Test directory change and relative path */
    printf("$ cd usr\n");
    path[0] = "cd";
    path[1] = "usr";
    path[2] = NULL;
    cmd_cd(path);

    printf("$ ls\n");
    path[0] = "ls";
    path[1] = NULL;
    cmd_ls(path);

    printf("$ ls /\n");
    path[1] = "/";
    path[2] = NULL;
    cmd_ls(path);
    
    /* Test return to root */
    printf("$ cd /\n");
    path[0] = "cd";
    path[1] = "/";
    path[2] = NULL;
    cmd_cd(path);

    printf("$ ls\n");
    path[0] = "ls";
    path[1] = NULL;
    cmd_ls(path);
    
    printf("\nTry reserving 12 blocks:\n");
    for (j = 0; j < 12; j++) {
        l = allocBlock();
        printf("Reserved block %ld\n", l);
    }

    printf("Remove blocks 0, 5, 6:\n");
    freeBlock(6);
    freeBlock(0);
    freeBlock(5);

 
    printf("Try reserving 3 blocks:\n");
    for (j = 0; j < 3; j++) {
        l = allocBlock();
        printf("Reserved block %ld\n", l);
    }

    printf("\n\nBlock reservation test complete.\n\n");

}
