#define __USE_XOPEN
#define _GNU_SOURCE

#include "dirtree.h"
#include "cmds.h"
#include "simsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <unistd.h>

struct file_loaddata {
    char *name;
    long filesize;
    time_t timestamp;
};



int main(int argc, char *argv[]) {
    
    /* the filesystem and block sizes, in bytes */
    long blk_size = 0;
    long fs_size = 0;
    
    int i; 
    
    for (i = 1; argv[i]; i++) {
        if (!strcmp(argv[i], "-b")) {
            /* User sets a value for the block size */
            if (argv[i+1]) {
                blk_size = atol(argv[i+1]);
                i++;
            } else {
                printf("\033[1m\033[33mWarning\033[0m: Provided flag %s without value\n", argv[i]);
            }
        } else if (!strcmp(argv[i], "-s")) {
            /* User has a custom filesystem capacity */
            if (argv[i+1]) {
                fs_size = atol(argv[i+1]);
                i++;
            } else
                printf("\033[1m\033[33mWarning\033[0m: Provided flag %s without value\n", argv[i]);
        } 
    }

    if (!blk_size) {
        /* Check whether or not a block size was given */
        printf("\033[1m\033[33mWarning\033[0m: Block size not specified; defaulting to 512B\n");
        blk_size = 512;
    } else {
        printf("Using block size of %ldB\n", blk_size);
    }
    
    if (!fs_size) {
        /* Check whether or not a filesystem capacity was given */
        printf("\033[1m\033[33mWarning\033[0m: Filesystem size not specified; defaulting to 64kB\n");
        fs_size = 65536;
    } else {
        printf("Using filesystem size of %ldB\n", fs_size);
    }

    /* Initialize the filesystem */
    init_filesystem(blk_size, fs_size);


    while (1) {
        char buff[64];
        char *cmd = malloc(sizeof(char));
        char **arg_vec;
        int n, len;
        
        len = 0;
        
        printf("\033[1m\033[32m" "oslab@IIT(BHU)\033[0m:\033[1m\033[34m");
        
        /* Show present path */
        arg_vec = pathVecOfTree(getWorkDirNode());
        for (n = 0; arg_vec[n]; n++)
            printf("%s%s", arg_vec[n], arg_vec[n+1] ? "/" : "");
        free_str_vec(arg_vec);

        printf("\033[0m$ ");
        fflush(stdout);

        /* Read from stdin */
        while ((n = read(0, buff, 64))) {
            char *tmp = (char*) realloc(cmd, (len+65)*sizeof(char));

            if (!tmp) {
                tmp = (char*) malloc((len+65) * sizeof(char));
                strcpy(tmp, cmd);
                free(cmd);
            }
            cmd = tmp;

            n = 0;
            while (n < 64) {
                if (buff[n] == '\n') {
                    cmd[len+n] = '\0';
                    break;
                }
                cmd[len+n] = buff[n];
                n++;
            }

            len += n;

            if (n < 64)
                break;
        }

        cmd[len] = '\0';

        /* Tokenize the command */
        arg_vec = str_to_vec(cmd, ' ');
        free(cmd);

        /* Run the command */
        cmd_exec(arg_vec);
        free_str_vec(arg_vec);

        

    }
    
}

